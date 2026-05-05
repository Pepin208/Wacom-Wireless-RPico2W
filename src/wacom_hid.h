#ifndef WACOM_HID_H
#define WACOM_HID_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

// Wacom report struct based on the Intuos / CTL-4100 properties
typedef struct {
    uint16_t x;           // 0–15200 typically for Intuos S
    uint16_t y;           // 0-9500
    uint16_t pressure;    // 0–4095
    int8_t   tilt_x;      // Unused on CTL-4100, but in struct
    int8_t   tilt_y;
    bool     tip;         // True if pen touches tablet
    bool     btn_side1;   // Lower pen button
    bool     btn_side2;   // Upper pen button
    bool     eraser;      // Inverted pen / eraser mode
    bool     express_keys[4]; // The 4 express keys on the tablet
    uint8_t  report_id;
    bool     in_proximity; // True if pen is in range
} wacom_report_t;

bool wacom_parse_report(const uint8_t *report, uint16_t len, wacom_report_t *out_report);

const uint8_t* wacom_get_report_descriptor(void);
uint16_t wacom_get_report_descriptor_len(void);

#endif
