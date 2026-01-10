#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold24pt7b.h> 
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold36pt7b.h>
#include <Fonts/FreeSansBold30pt7b.h>
#include "pins_xiao-esp32c6.h"

// ESP32-C6 CS(SS)=21,SCL(SCK)=19,SDA(MOSI)=18,BUSY=3,RES(RST)=2,DC=1
#define CS_PIN XIAO_SS
#define BUSY_PIN 2
#define RES_PIN 1
#define DC_PIN 0

int8_t tempDir = 0; // -1 down, 0 same/unknown, +1 up
int8_t humDir  = 0;

// 1.54'' EPD Module
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(/*CS=21*/ CS_PIN, /*DC=*/ DC_PIN, /*RES=*/ RES_PIN, /*BUSY=*/ BUSY_PIN)); // GDEH0154D67 200x200, SSD1681

void init_display()
{
  pinMode(CS_PIN, OUTPUT);
  pinMode(DC_PIN, OUTPUT);
  pinMode(RES_PIN, OUTPUT);
  pinMode(BUSY_PIN, INPUT);
  display.init(115200, true, 50, false);
}

void drawTrendArrow(int16_t cx, int16_t cy, int8_t dir, int16_t s = 7)
{
  // cx,cy = arrow center; s = size
  if (dir > 0)
  {
    // Up triangle
    display.fillTriangle(cx, cy - s,  cx - s, cy + s,  cx + s, cy + s, GxEPD_BLACK);
  }
  else if (dir < 0)
  {
    // Down triangle
    display.fillTriangle(cx, cy + s,  cx - s, cy - s,  cx + s, cy - s, GxEPD_BLACK);
  }
  else
  {
    // Neutral: small line
    display.drawLine(cx - s, cy, cx + s, cy, GxEPD_BLACK);
  }
}

void drawTemperatureText(float tempC, int8_t dir)
{
  display.setFont(&FreeSansBold30pt7b);
  display.setTextColor(GxEPD_BLACK);
  drawTrendArrow(15, 50, dir, 7);

  char tempStr[10];
  dtostrf(tempC, 4, 1, tempStr);

  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(tempStr, 0, 0, &tbx, &tby, &tbw, &tbh);

  display.setCursor(80 - tbw / 2, 65);
  display.print(tempStr);

  // degree symbol and unit
  display.setFont(&FreeSansBold12pt7b);
  display.setCursor(80 + tbw / 2 + 4, 45);
  display.print("o");
  display.setFont(&FreeSansBold30pt7b);
  display.setCursor(80 + tbw / 2 + 20, 65);
  display.print("C");
}

void drawHumidityText(float hum, int8_t dir)
{
  display.setFont(&FreeSansBold30pt7b);
  display.setTextColor(GxEPD_BLACK);
  drawTrendArrow(15, 150, dir, 7);

  char humStr[10];
  dtostrf(hum, 4, 1, humStr);

  int16_t tbx, tby;
  uint16_t tbw, tbh;
  display.getTextBounds(humStr, 0, 0, &tbx, &tby, &tbw, &tbh);

  display.setCursor(80 - tbw / 2, 150 + tbh / 2);
  display.print(humStr);
  display.print("%");
}

void drawTHFull(float tempC, float hum)
{
  display.setRotation(1);
  const uint16_t w = display.width();
  const uint16_t h = display.height();
  const uint16_t lineY = h / 2;

  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.drawLine(0, lineY, w, lineY, GxEPD_BLACK);
    drawTemperatureText(tempC, tempDir);
    drawHumidityText(hum, humDir);
  } while (display.nextPage());
}

void drawTempPartial(float tempC)
{
  display.setRotation(1);
  const uint16_t w = display.width();
  const uint16_t h = display.height();
  const uint16_t lineY = h / 2;
  const uint16_t margin = 6; // don't overwrite the divider line

  const uint16_t winX = 0;
  const uint16_t winY = 0;
  const uint16_t winW = w;
  const uint16_t winH = lineY - margin;

  display.setPartialWindow(winX, winY, winW, winH);
  display.firstPage();
  do
  {
    display.fillRect(winX, winY, winW, winH, GxEPD_WHITE);
    drawTemperatureText(tempC, tempDir);
  } while (display.nextPage());
}

void drawHumPartial(float hum)
{
  display.setRotation(1);
  const uint16_t w = display.width();
  const uint16_t h = display.height();
  const uint16_t lineY = h / 2;
  const uint16_t margin = 6; // don't overwrite the divider line

  const uint16_t winX = 0;
  const uint16_t winY = lineY + margin;
  const uint16_t winW = w;
  const uint16_t winH = (winY < h) ? (h - winY) : 0;

  display.setPartialWindow(winX, winY, winW, winH);
  display.firstPage();
  do
  {
    display.fillRect(winX, winY, winW, winH, GxEPD_WHITE);
    drawHumidityText(hum, humDir);
  } while (display.nextPage());
}
