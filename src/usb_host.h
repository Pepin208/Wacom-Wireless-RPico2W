#ifndef USB_HOST_H
#define USB_HOST_H

#include <stdint.h>
#include <stdbool.h>

void usb_host_init(void);
void usb_host_task(void);

// Callback function type to process new Wacom reports
typedef void (*usb_report_cb_t)(const uint8_t *report, uint16_t len);

// Set the callback that will receive raw USB HID reports
void usb_host_set_report_callback(usb_report_cb_t cb);

#endif
