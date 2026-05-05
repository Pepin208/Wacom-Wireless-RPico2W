#include "config.h"

#if TRANSPORT_MODE == TRANSPORT_BLE

#include "transport_ble.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "btstack.h"
#include <string.h>

// BLE HID Service setup
static btstack_packet_callback_registration_t hci_event_callback_registration;
static hci_con_handle_t hid_cid = 0;

static const uint8_t adv_data[] = {
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,
    0x0F, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'W', 'a', 'c', 'o', 'm', ' ', 'W', 'i', 'r', 'e', 'l', 'e', 's', 's',
    0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE & 0xff, ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE >> 8,
    0x03, BLUETOOTH_DATA_TYPE_APPEARANCE, 0xC5, 0x03,
};

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    if (packet_type != HCI_EVENT_PACKET) return;

    switch (hci_event_packet_get_type(packet)) {
        case HCI_EVENT_DISCONNECTION_COMPLETE:
            hid_cid = 0;
            printf("BLE Disconnected\n");
            break;
        case HCI_EVENT_LE_META:
            switch (hci_event_le_meta_get_subevent_code(packet)) {
                case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
                    hid_cid = hci_subevent_le_connection_complete_get_connection_handle(packet);
                    printf("BLE Connected, handle 0x%04x\n", hid_cid);
                    break;
            }
            break;
    }
}

// Dummy GATT database for standalone compiling without running btstack compiler
static const uint8_t profile_data[] = {
    // 0x0001 PRIMARY_SERVICE-GAP_SERVICE
    0x0a, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x28, 0x00, 0x18,
    // 0x0002 CHARACTERISTIC-GAP_DEVICE_NAME-READ
    0x0d, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x28, 0x02, 0x03, 0x00, 0x00, 0x2a,
    // 0x0003 VALUE-GAP_DEVICE_NAME-READ-'Wacom Wireless'
    0x16, 0x00, 0x02, 0x00, 0x03, 0x00, 0x00, 0x2a, 0x57, 0x61, 0x63, 0x6f, 0x6d, 0x20, 0x57, 0x69, 0x72, 0x65, 0x6c, 0x65, 0x73, 0x73,
    // 0x0004 CHARACTERISTIC-GAP_APPEARANCE-READ
    0x0d, 0x00, 0x02, 0x00, 0x04, 0x00, 0x03, 0x28, 0x02, 0x05, 0x00, 0x01, 0x2a,
    // 0x0005 VALUE-GAP_APPEARANCE-READ-
    0x0a, 0x00, 0x02, 0x00, 0x05, 0x00, 0x01, 0x2a, 0xc5, 0x03,
    // 0x0006 PRIMARY_SERVICE-GATT_SERVICE
    0x0a, 0x00, 0x02, 0x00, 0x06, 0x00, 0x00, 0x28, 0x01, 0x18,
    // 0x0007 PRIMARY_SERVICE-HUMAN_INTERFACE_DEVICE_SERVICE
    0x0a, 0x00, 0x02, 0x00, 0x07, 0x00, 0x00, 0x28, 0x12, 0x18,
    // 0x0008 CHARACTERISTIC-HID_INFORMATION-READ
    0x0d, 0x00, 0x02, 0x00, 0x08, 0x00, 0x03, 0x28, 0x02, 0x09, 0x00, 0x4a, 0x2a,
    // 0x0009 VALUE-HID_INFORMATION-READ-
    0x0c, 0x00, 0x02, 0x00, 0x09, 0x00, 0x4a, 0x2a, 0x01, 0x01, 0x00, 0x02,
    // 0x000a CHARACTERISTIC-HID_REPORT_MAP-READ
    0x0d, 0x00, 0x02, 0x00, 0x0a, 0x00, 0x03, 0x28, 0x02, 0x0b, 0x00, 0x4b, 0x2a,
    // 0x000b VALUE-HID_REPORT_MAP-READ-
    0x09, 0x00, 0x02, 0x00, 0x0b, 0x00, 0x4b, 0x2a, 0x00,
    // 0x000c CHARACTERISTIC-HID_CONTROL_POINT-WRITE_WITHOUT_RESPONSE
    0x0d, 0x00, 0x02, 0x00, 0x0c, 0x00, 0x03, 0x28, 0x04, 0x0d, 0x00, 0x4c, 0x2a,
    // 0x000d VALUE-HID_CONTROL_POINT-WRITE_WITHOUT_RESPONSE-
    0x09, 0x00, 0x04, 0x00, 0x0d, 0x00, 0x4c, 0x2a, 0x00,
    // 0x000e CHARACTERISTIC-HID_REPORT-READ | NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x0e, 0x00, 0x03, 0x28, 0x12, 0x0f, 0x00, 0x4d, 0x2a,
    // 0x000f VALUE-HID_REPORT-READ | NOTIFY-
    0x09, 0x00, 0x02, 0x00, 0x0f, 0x00, 0x4d, 0x2a, 0x00,
    // 0x0010 CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x10, 0x00, 0x02, 0x29, 0x00, 0x00,
    // 0x0011 REPORT_REFERENCE
    0x0a, 0x00, 0x02, 0x00, 0x11, 0x00, 0x08, 0x29, 0x10, 0x01,
    // 0x0012 CHARACTERISTIC-HID_REPORT-READ | NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x12, 0x00, 0x03, 0x28, 0x12, 0x13, 0x00, 0x4d, 0x2a,
    // 0x0013 VALUE-HID_REPORT-READ | NOTIFY-
    0x09, 0x00, 0x02, 0x00, 0x13, 0x00, 0x4d, 0x2a, 0x00,
    // 0x0014 CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x14, 0x00, 0x02, 0x29, 0x00, 0x00,
    // 0x0015 REPORT_REFERENCE
    0x0a, 0x00, 0x02, 0x00, 0x15, 0x00, 0x08, 0x29, 0x11, 0x01,
    0x00, 0x00
};

static uint16_t att_read_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size) {
    if (att_handle == 0x000b) {
        // HID_REPORT_MAP
        return att_read_callback_handle_blob(wacom_get_report_descriptor(), wacom_get_report_descriptor_len(), offset, buffer, buffer_size);
    }
    return 0;
}

static int att_write_callback(hci_con_handle_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size) {
    return 0;
}

void ble_transport_init(void) {
    if (cyw43_arch_init()) {
        printf("CYW43 arch init failed\n");
        return;
    }

    l2cap_init();
    sm_init();
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION | SM_AUTHREQ_BONDING);

    att_server_init(profile_data, att_read_callback, att_write_callback);

    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // Setup Advertising
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr;
    memset(null_addr, 0, 6);
    gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data(sizeof(adv_data), (uint8_t*)adv_data);
    gap_advertisements_enable(1);

    hci_power_control(HCI_POWER_ON);
}

void ble_transport_task(void) {
    // Handled in background by cyw43 / btstack internals
}

void ble_transport_send_report(wacom_report_t *report) {
    if (hid_cid == 0) return;

    uint8_t pen_buf[27] = {0};

    if (report->report_id == 0x10) {
        if (report->in_proximity) pen_buf[0] |= 0x20;
        if (report->eraser)       pen_buf[0] |= 0x10;
        if (report->btn_side2)    pen_buf[0] |= 0x04;
        if (report->btn_side1)    pen_buf[0] |= 0x02;
        if (report->tip)          pen_buf[0] |= 0x01;

        pen_buf[1] = (report->x) & 0xFF;
        pen_buf[2] = (report->x >> 8) & 0xFF;

        pen_buf[3] = (report->y) & 0xFF;
        pen_buf[4] = (report->y >> 8) & 0xFF;

        pen_buf[5] = (report->pressure) & 0xFF;
        pen_buf[6] = (report->pressure >> 8) & 0xFF;

        pen_buf[7] = report->tilt_x;
        pen_buf[8] = report->tilt_y;

        att_server_notify(hid_cid, 0x000f, pen_buf, 26);
    } else if (report->report_id == 0x11) {
        uint8_t aux_buf[26] = {0};
        if (report->express_keys[0]) aux_buf[0] |= 0x01;
        if (report->express_keys[1]) aux_buf[0] |= 0x02;
        if (report->express_keys[2]) aux_buf[0] |= 0x04;
        if (report->express_keys[3]) aux_buf[0] |= 0x08;
        att_server_notify(hid_cid, 0x0013, aux_buf, 26);
    }
}

#endif
