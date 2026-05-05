#ifndef CONFIG_H
#define CONFIG_H

#define TRANSPORT_BLE   0
#define TRANSPORT_WIFI  1

// --- Transport Selection ---
#define TRANSPORT_MODE  TRANSPORT_BLE  // change to TRANSPORT_WIFI for WiFi mode

// --- WiFi & UDP Configuration ---
#define WIFI_SSID "your_ssid"
#define WIFI_PASS "your_password"
#define DAEMON_IP "255.255.255.255"  // override with specific PC IP
#define DAEMON_PORT 9876

// --- Buffer Sizes (Static, No VLAs) ---
// Max raw report size expected from CTL-4100 is 27 bytes.
#define WACOM_RAW_REPORT_SIZE 32
// Serialized wacom_report_t size
// 18 bytes for little-endian structs usually
#define UDP_PACKET_SIZE 18

// --- Device IDs ---
#define WACOM_VID  0x056A
#define WACOM_PID  0x0374

#endif
