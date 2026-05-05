#ifndef TRANSPORT_BLE_H
#define TRANSPORT_BLE_H

#include "wacom_hid.h"

void ble_transport_init(void);
void ble_transport_task(void);
void ble_transport_send_report(wacom_report_t *report);

#endif
