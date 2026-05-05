#ifndef TRANSPORT_WIFI_H
#define TRANSPORT_WIFI_H

#include "wacom_hid.h"

void wifi_transport_init(void);
void wifi_transport_task(void);
void wifi_transport_send_report(wacom_report_t *report);

#endif
