#include "wacom_hid.h"
#include <string.h>

bool wacom_parse_report(const uint8_t *report, uint16_t len, wacom_report_t *out_report) {
    if (len < 10) return false;

    // Based on Wacom Intuos V2 / CTL-4100 structure
    uint8_t report_id = report[0];
    out_report->report_id = report_id;

    if (report_id == 0x10) { // Pen report
        if (len < 12) return false;

        // Report[1] holds pen status
        uint8_t pen_byte = report[1];

        out_report->in_proximity = (pen_byte & 0x20) != 0; // Bit 5
        out_report->eraser = (pen_byte & 0x10) != 0;       // Bit 4
        out_report->btn_side2 = (pen_byte & 0x04) != 0;    // Bit 2
        out_report->btn_side1 = (pen_byte & 0x02) != 0;    // Bit 1
        out_report->tip = (pen_byte & 0x01) != 0;          // Bit 0

        // Position X: bytes 2 and 3 (16-bit little-endian)
        out_report->x = report[2] | (report[3] << 8);

        // Position Y: bytes 5 and 6 (16-bit little-endian)
        out_report->y = report[5] | (report[6] << 8);

        // Pressure: bytes 8 and 9 (16-bit little-endian)
        out_report->pressure = report[8] | (report[9] << 8);

        // Tilt X/Y
        out_report->tilt_x = (int8_t)report[10];
        out_report->tilt_y = (int8_t)report[11];

        return true;
    } else if (report_id == 0x11) { // Aux/Express Keys report
        // Report[1] holds express key status
        uint8_t aux_byte = report[1];
        out_report->express_keys[0] = (aux_byte & 0x01) != 0;
        out_report->express_keys[1] = (aux_byte & 0x02) != 0;
        out_report->express_keys[2] = (aux_byte & 0x04) != 0;
        out_report->express_keys[3] = (aux_byte & 0x08) != 0;
        return true;
    }

    return false;
}

// Emulating the exact HID report descriptor that OpenTabletDriver expects for CTL-4100.
// This wraps the Digitizer usage page for BLE HID profile compatibility.
static const uint8_t wacom_report_descriptor[] = {
    // ----------------------
    // PEN REPORT (ID 0x10)
    // ----------------------
    0x05, 0x0D,                         // Usage Page (Digitizer)
    0x09, 0x02,                         // Usage (Pen)
    0xA1, 0x01,                         // Collection (Application)
    0x85, 0x10,                         //   Report ID (16)
    0x09, 0x20,                         //   Usage (Stylus)
    0xA1, 0x00,                         //   Collection (Physical)

    // Pen buttons and status (Byte 1)
    0x09, 0x42,                         //     Usage (Tip Switch)
    0x09, 0x44,                         //     Usage (Barrel Switch)
    0x09, 0x45,                         //     Usage (Eraser)
    0x09, 0x3C,                         //     Usage (Invert)
    0x09, 0x32,                         //     Usage (In Range)
    0x15, 0x00,                         //     Logical Minimum (0)
    0x25, 0x01,                         //     Logical Maximum (1)
    0x75, 0x01,                         //     Report Size (1)
    0x95, 0x05,                         //     Report Count (5)
    0x81, 0x02,                         //     Input (Data, Variable, Absolute)
    0x95, 0x03,                         //     Report Count (3) - Padding
    0x81, 0x03,                         //     Input (Constant, Variable, Absolute)

    // X, Y coordinates (Bytes 2-7) - sending 16 bits to match struct
    0x05, 0x01,                         //     Usage Page (Generic Desktop)
    0x09, 0x30,                         //     Usage (X)
    0x15, 0x00,                         //     Logical Minimum (0)
    0x26, 0x60, 0x3B,                   //     Logical Maximum (15200)
    0x75, 0x10,                         //     Report Size (16)
    0x95, 0x01,                         //     Report Count (1)
    0x81, 0x02,                         //     Input (Data, Variable, Absolute)

    // Padding 1 byte since the original struct has 24 bits but we use 16 bits here for our mock
    0x75, 0x08, 0x95, 0x01, 0x81, 0x03,

    0x09, 0x31,                         //     Usage (Y)
    0x26, 0x1C, 0x25,                   //     Logical Maximum (9500)
    0x75, 0x10,                         //     Report Size (16)
    0x81, 0x02,                         //     Input (Data, Variable, Absolute)

    // Padding 1 byte
    0x75, 0x08, 0x95, 0x01, 0x81, 0x03,

    // Pressure (Bytes 8-9) - 16 bits
    0x05, 0x0D,                         //     Usage Page (Digitizer)
    0x09, 0x30,                         //     Usage (Tip Pressure)
    0x26, 0xFF, 0x0F,                   //     Logical Maximum (4095)
    0x75, 0x10,                         //     Report Size (16)
    0x95, 0x01,                         //     Report Count (1)
    0x81, 0x02,                         //     Input (Data, Variable, Absolute)

    // Tilt X, Y (Bytes 10-11) - 8 bits each
    0x09, 0x3D,                         //     Usage (X Tilt)
    0x09, 0x3E,                         //     Usage (Y Tilt)
    0x15, 0x81,                         //     Logical Minimum (-127)
    0x25, 0x7F,                         //     Logical Maximum (127)
    0x75, 0x08,                         //     Report Size (8)
    0x95, 0x02,                         //     Report Count (2)
    0x81, 0x02,                         //     Input (Data, Variable, Absolute)

    // Padding (Bytes 12-26) to make it 27 bytes total
    0x75, 0x08,                         //     Report Size (8)
    0x95, 0x0F,                         //     Report Count (15)
    0x81, 0x03,                         //     Input (Constant, Variable, Absolute)
    0xC0,                               //   End Collection
    0xC0,                               // End Collection

    // ----------------------
    // PAD REPORT (ID 0x11)
    // ----------------------
    0x05, 0x01,                         // Usage Page (Generic Desktop)
    0x09, 0x0E,                         // Usage (System Multi-Axis Controller)
    0xA1, 0x01,                         // Collection (Application)
    0x85, 0x11,                         //   Report ID (17)

    // Express Keys (Byte 1)
    0x05, 0x09,                         //   Usage Page (Button)
    0x19, 0x01,                         //   Usage Minimum (1)
    0x29, 0x04,                         //   Usage Maximum (4)
    0x15, 0x00,                         //   Logical Minimum (0)
    0x25, 0x01,                         //   Logical Maximum (1)
    0x75, 0x01,                         //   Report Size (1)
    0x95, 0x04,                         //   Report Count (4)
    0x81, 0x02,                         //   Input (Data, Variable, Absolute)

    // Padding to complete byte
    0x95, 0x04,                         //   Report Count (4)
    0x81, 0x03,                         //   Input (Constant, Variable, Absolute)

    // Padding for the rest of the report (Bytes 2-26)
    0x75, 0x08,                         //   Report Size (8)
    0x95, 0x19,                         //   Report Count (25)
    0x81, 0x03,                         //   Input (Constant, Variable, Absolute)
    0xC0                                // End Collection
};

const uint8_t* wacom_get_report_descriptor(void) {
    return wacom_report_descriptor;
}

uint16_t wacom_get_report_descriptor_len(void) {
    return sizeof(wacom_report_descriptor);
}
