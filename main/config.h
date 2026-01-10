#pragma once
#include <stdint.h>

// E-Ink display update behavior
static const float TEMP_DELTA_C = 0.1f; // temperature change to trigger update           
static const float HUM_DELTA_PCT = 3.0f; // humidity change to trigger update         
static const uint8_t FULL_REFRESH_EVERY_N_CHANGES = 30; // to avoid ghosting

static const uint32_t SENSOR_READ_INTERVAL_MS = 2000; //temporary

// Matter
const uint32_t decommissioningTimeoutMs = 5000;  // keep the button pressed for 5s, or longer, to decommission
const uint16_t MatterUpdateIntervalMs = 5000; // update Matter every 5 seconds