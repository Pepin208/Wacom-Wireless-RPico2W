#include "config.h"

#if TRANSPORT_MODE == TRANSPORT_WIFI

#include "transport_wifi.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include <string.h>
#include <stdio.h>

static struct udp_pcb *udp_conn = NULL;
static ip_addr_t target_ip;
static bool wifi_connected = false;
static bool connect_in_progress = false;

static void wifi_connect_task(void) {
    if (connect_in_progress) return;

    int status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
    if (status == CYW43_LINK_UP) {
        wifi_connected = true;
        printf("Wi-Fi connected.\n");

        if (udp_conn != NULL) {
            udp_remove(udp_conn);
        }
        udp_conn = udp_new();

        if (ipaddr_aton(DAEMON_IP, &target_ip) == 0) {
            printf("Invalid daemon IP\n");
        }
    } else if (status < 0 || status == CYW43_LINK_DOWN) {
        connect_in_progress = true;
        printf("Connecting to Wi-Fi SSID: %s\n", WIFI_SSID);
        cyw43_arch_wifi_connect_async(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK);
        connect_in_progress = false;
    }
}

void wifi_transport_init(void) {
    if (cyw43_arch_init()) {
        printf("CYW43 arch init failed\n");
        return;
    }

    cyw43_arch_enable_sta_mode();
}

void wifi_transport_task(void) {
    if (!wifi_connected || cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) != CYW43_LINK_UP) {
        if (wifi_connected) {
            printf("Wi-Fi disconnected.\n");
            wifi_connected = false;
        }

        static uint32_t last_reconnect_time = 0;
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_reconnect_time > 5000) {
            last_reconnect_time = now;
            wifi_connect_task();
        }
    }
}

void wifi_transport_send_report(wacom_report_t *report) {
    if (!wifi_connected || udp_conn == NULL) return;

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, UDP_PACKET_SIZE, PBUF_RAM);
    if (!p) {
        printf("Failed to allocate pbuf for UDP\n");
        return;
    }

    memcpy(p->payload, report, UDP_PACKET_SIZE);

    udp_sendto(udp_conn, p, &target_ip, DAEMON_PORT);
    pbuf_free(p);
}

#endif
