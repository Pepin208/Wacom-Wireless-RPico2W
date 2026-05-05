#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define DAEMON_PORT 9876

// Matching the wacom_report_t structure from the firmware
typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t pressure;
    int8_t   tilt_x;
    int8_t   tilt_y;
    bool     tip;
    bool     btn_side1;
    bool     btn_side2;
    bool     eraser;
    bool     express_keys[4];
    uint8_t  report_id;
    bool     in_proximity;
} wacom_report_t;

/* ----------------------------------------------------------
 * VIRTUAL HID INJECTION — USER MUST IMPLEMENT
 * ----------------------------------------------------------
 * Depending on your virtual HID driver, implement this function:
 *
 * Option A — VMulti:
 *   Include vmulti client headers and call vmulti_update_digitizer()
 *   with the parsed coordinates and pressure.
 *
 * Option B — ViGEm / VirtualHID:
 *   Use the appropriate API to inject a raw HID report.
 *
 * Option C — Raw HID via custom driver:
 *   WriteFile() to the virtual device handle with the raw report.
 *
 * The struct passed in contains all parsed Wacom data ready to use.
 * ----------------------------------------------------------
 */
static void inject_hid_report(const wacom_report_t *report) {
    /* TODO: implement using your virtual HID driver of choice */
    (void)report; /* suppress unused warning until implemented */
}

int main() {
    WSADATA wsaData;
    SOCKET recvSocket;
    struct sockaddr_in recvAddr;
    struct sockaddr_in senderAddr;
    int senderAddrSize = sizeof(senderAddr);
    wacom_report_t report;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        return 1;
    }

    printf("Virtual HID driver hook ready.\n");

    recvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (recvSocket == INVALID_SOCKET) {
        printf("Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(DAEMON_PORT);
    recvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(recvSocket, (SOCKADDR*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR) {
        printf("Bind failed.\n");
        closesocket(recvSocket);
        WSACleanup();
        return 1;
    }

    printf("Wacom WiFi Daemon listening on UDP port %d (Windows)\n", DAEMON_PORT);

    while (1) {
        int bytesReceived = recvfrom(recvSocket, (char*)&report, sizeof(report), 0, (SOCKADDR*)&senderAddr, &senderAddrSize);
        if (bytesReceived == sizeof(report)) {
            inject_hid_report(&report);
        }
    }

    closesocket(recvSocket);
    WSACleanup();
    return 0;
}
