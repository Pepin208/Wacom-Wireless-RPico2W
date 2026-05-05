#include "usb_host.h"
#include "tusb.h"
#include "config.h"

static usb_report_cb_t report_cb = NULL;

void usb_host_init(void) {
    tusb_init();
}

void usb_host_task(void) {
    tuh_task();
}

void usb_host_set_report_callback(usb_report_cb_t cb) {
    report_cb = cb;
}

//--------------------------------------------------------------------+
// TinyUSB Host HID Callbacks
//--------------------------------------------------------------------+

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    uint16_t vid, pid;
    tuh_vid_pid_get(dev_addr, &vid, &pid);

    if (vid == WACOM_VID && pid == WACOM_PID) {
        tuh_hid_set_protocol(dev_addr, instance, HID_PROTOCOL_REPORT);

        if (!tuh_hid_receive_report(dev_addr, instance)) {
            // Error handling could be added here
        }
    }
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
    // Handle unmount if needed
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    uint16_t vid, pid;
    tuh_vid_pid_get(dev_addr, &vid, &pid);

    if (vid == WACOM_VID && pid == WACOM_PID) {
        if (report_cb) {
            report_cb(report, len);
        }
    }

    tuh_hid_receive_report(dev_addr, instance);
}
