#pragma once
#include <stdint.h>

static const uint8_t LED_BUILTIN = 15;
#define BUILTIN_LED LED_BUILTIN  // backward compatibility
#define LED_BUILTIN LED_BUILTIN  // allow testing #ifdef LED_BUILTIN

static const uint8_t TX = 16;
static const uint8_t RX = 17;

static const uint8_t SDA = 22;
static const uint8_t SCL = 23;

static const uint8_t SS = 21;
static const uint8_t MOSI = 18;
static const uint8_t MISO = 20;
static const uint8_t SCK = 19;

static const uint8_t A0 = 0;
static const uint8_t A1 = 1;
static const uint8_t A2 = 2;

static const uint8_t D0 = 0;
static const uint8_t D1 = 1;
static const uint8_t D2 = 2;
static const uint8_t D3 = 21;
static const uint8_t D4 = 22;
static const uint8_t D5 = 23;
static const uint8_t D6 = 16;
static const uint8_t D7 = 17;
static const uint8_t D8 = 19;
static const uint8_t D9 = 20;
static const uint8_t D10 = 18;

static const uint8_t WIFI_ENABLE = 3;
static const uint8_t WIFI_ANT_CONFIG = 14;