#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/uhid.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/select.h>

#define DAEMON_PORT 9876

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

static const uint8_t wacom_report_descriptor[] = {
    0x05, 0x0D, 0x09, 0x02, 0xA1, 0x01, 0x85, 0x10,
    0x09, 0x20, 0xA1, 0x00, 0x09, 0x42, 0x09, 0x44,
    0x09, 0x45, 0x09, 0x3C, 0x09, 0x32, 0x15, 0x00,
    0x25, 0x01, 0x75, 0x01, 0x95, 0x05, 0x81, 0x02,
    0x95, 0x03, 0x81, 0x03, 0x05, 0x01, 0x09, 0x30,
    0x15, 0x00, 0x26, 0x60, 0x3B, 0x75, 0x10, 0x95,
    0x01, 0x81, 0x02, 0x75, 0x08, 0x95, 0x01, 0x81,
    0x03, 0x09, 0x31, 0x26, 0x1C, 0x25, 0x75, 0x10,
    0x81, 0x02, 0x75, 0x08, 0x95, 0x01, 0x81, 0x03,
    0x05, 0x0D, 0x09, 0x30, 0x26, 0xFF, 0x0F, 0x75,
    0x10, 0x95, 0x01, 0x81, 0x02, 0x09, 0x3D, 0x09,
    0x3E, 0x15, 0x81, 0x25, 0x7F, 0x75, 0x08, 0x95,
    0x02, 0x81, 0x02, 0x75, 0x08, 0x95, 0x0F, 0x81,
    0x03, 0xC0, 0xC0, 0x05, 0x01, 0x09, 0x0E, 0xA1,
    0x01, 0x85, 0x11, 0x05, 0x09, 0x19, 0x01, 0x29,
    0x04, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95,
    0x04, 0x81, 0x02, 0x95, 0x04, 0x81, 0x03, 0x75,
    0x08, 0x95, 0x19, 0x81, 0x03, 0xC0
};

static int uhid_fd = -1;

void uhid_write(struct uhid_event *ev) {
    if (write(uhid_fd, ev, sizeof(*ev)) < 0) {
        perror("write uhid");
    }
}

void setup_uhid() {
    uhid_fd = open("/dev/uhid", O_RDWR | O_CLOEXEC);
    if (uhid_fd < 0) {
        perror("Cannot open /dev/uhid");
        exit(EXIT_FAILURE);
    }

    struct uhid_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = UHID_CREATE2;
    strncpy((char*)ev.u.create2.name, "Wacom CTL-4100 (WiFi)", sizeof(ev.u.create2.name) - 1);
    strncpy((char*)ev.u.create2.phys, "mac:00:11:22", sizeof(ev.u.create2.phys) - 1);

    memcpy(ev.u.create2.rd_data, wacom_report_descriptor, sizeof(wacom_report_descriptor));
    ev.u.create2.rd_size = sizeof(wacom_report_descriptor);
    ev.u.create2.bus = BUS_USB;
    ev.u.create2.vendor = 0x056A;
    ev.u.create2.product = 0x0374;
    ev.u.create2.version = 0x0100;
    ev.u.create2.country = 0;

    uhid_write(&ev);
}

void process_report(wacom_report_t *report) {
    struct uhid_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.type = UHID_INPUT2;

    if (report->report_id == 0x10) {
        uint8_t pen_buf[27] = {0};
        pen_buf[0] = 0x10;
        if (report->in_proximity) pen_buf[1] |= 0x20;
        if (report->eraser)       pen_buf[1] |= 0x10;
        if (report->btn_side2)    pen_buf[1] |= 0x04;
        if (report->btn_side1)    pen_buf[1] |= 0x02;
        if (report->tip)          pen_buf[1] |= 0x01;

        pen_buf[2] = (report->x) & 0xFF;
        pen_buf[3] = (report->x >> 8) & 0xFF;

        pen_buf[4] = (report->y) & 0xFF;
        pen_buf[5] = (report->y >> 8) & 0xFF;

        pen_buf[6] = (report->pressure) & 0xFF;
        pen_buf[7] = (report->pressure >> 8) & 0xFF;

        pen_buf[8] = report->tilt_x;
        pen_buf[9] = report->tilt_y;

        ev.u.input2.size = 27;
        memcpy(ev.u.input2.data, pen_buf, 27);
        uhid_write(&ev);
    } else if (report->report_id == 0x11) {
        uint8_t aux_buf[27] = {0};
        aux_buf[0] = 0x11;
        if (report->express_keys[0]) aux_buf[1] |= 0x01;
        if (report->express_keys[1]) aux_buf[1] |= 0x02;
        if (report->express_keys[2]) aux_buf[1] |= 0x04;
        if (report->express_keys[3]) aux_buf[1] |= 0x08;

        ev.u.input2.size = 27;
        memcpy(ev.u.input2.data, aux_buf, 27);
        uhid_write(&ev);
    }
}

int main() {
    setup_uhid();

    int sockfd;
    struct sockaddr_in servaddr, cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(DAEMON_PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    wacom_report_t report;
    socklen_t len = sizeof(cliaddr);

    printf("Wacom WiFi Daemon listening on UDP port %d\n", DAEMON_PORT);

    int max_fd = (sockfd > uhid_fd) ? sockfd : uhid_fd;

    while (1) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sockfd, &fds);
        FD_SET(uhid_fd, &fds);

        int ret = select(max_fd + 1, &fds, NULL, NULL, NULL);
        if (ret < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(uhid_fd, &fds)) {
            struct uhid_event ev;
            ssize_t ret_read = read(uhid_fd, &ev, sizeof(ev));
            if (ret_read > 0) {
                switch (ev.type) {
                    case UHID_START:
                    case UHID_OPEN:
                    case UHID_STOP:
                    case UHID_CLOSE:
                        // Acknowledge these state changes if necessary or just log
                        break;
                    default:
                        break;
                }
            }
        }

        if (FD_ISSET(sockfd, &fds)) {
            int n = recvfrom(sockfd, &report, sizeof(report), 0, (struct sockaddr *)&cliaddr, &len);
            if (n == sizeof(report)) {
                process_report(&report);
            }
        }
    }

    return 0;
}
