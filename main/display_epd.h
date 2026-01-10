#pragma once
#include <stdint.h>

void display_init();
void display_show_full(float tempC, float humPct, int8_t tempDir, int8_t humDir);
void display_show_partial_temp(float tempC, int8_t tempDir);
void display_show_partial_hum(float humPct, int8_t humDir);
