#pragma once
#include <stdint.h>

void init_display();
void drawTHFull(float tempC, float hum);
void drawTempPartial(float tempC);
void drawHumPartial(float hum);

extern int8_t tempDir; // -1 down, 0 same/unknown, +1 up
extern int8_t humDir;