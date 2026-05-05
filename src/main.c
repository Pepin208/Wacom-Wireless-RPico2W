#include <stdio.h>
#include "pico/stdlib.h"
#include "usb_host.h"
#include "wacom_hid.h"
#include "config.h"

#if TRANSPORT_MODE == TRANSPORT_BLE
#include "transport_ble.h"
#elif TRANSPORT_MODE == TRANSPORT_WIFI
#include "transport_wifi.h"
#endif

// Forward parsed reports to the active transport layer
static void raw_report_received(const uint8_t *raw_report, uint16_t len) {
    wacom_report_t parsed_report = {0};

    if (wacom_parse_report(raw_report, len, &parsed_report)) {
#if TRANSPORT_MODE == TRANSPORT_BLE
        ble_transport_send_report(&parsed_report);
#elif TRANSPORT_MODE == TRANSPORT_WIFI
        wifi_transport_send_report(&parsed_report);
#endif
    }
}

int main(void) {
    // Initialize stdio (USB/UART)
    stdio_init_all();
    printf("Wacom Wireless Starting...\n");

    // Initialize TinyUSB Host
    usb_host_init();
    usb_host_set_report_callback(raw_report_received);

    // Initialize Active Transport
#if TRANSPORT_MODE == TRANSPORT_BLE
    printf("Mode: BLE\n");
    ble_transport_init();
#elif TRANSPORT_MODE == TRANSPORT_WIFI
    printf("Mode: WiFi (UDP)\n");
    wifi_transport_init();
#endif

    // Main Loop
    while (true) {
        // Service USB Host
        usb_host_task();

        // Service Active Transport
#if TRANSPORT_MODE == TRANSPORT_BLE
        ble_transport_task();
#elif TRANSPORT_MODE == TRANSPORT_WIFI
        wifi_transport_task();
#endif
    }

    return 0;
}
