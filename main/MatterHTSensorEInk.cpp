#include <Arduino.h>
#include <Matter.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <math.h>

#include "pins_xiao-esp32c6.h"
#include "display_epd.h"
#include "config.h"

// List of Matter Endpoints for this Node
// Matter Humidity and Temperature Sensor Endpoint
MatterTemperatureSensor TemperatureSensor;
MatterHumiditySensor HumiditySensor;

// set your board USER BUTTON pin here - decommissioning button
const uint8_t buttonPin = BOOT_PIN;  // Set your pin here. Using BOOT Button.

Adafruit_AHTX0 aht;
static bool aht_ok = false;

// Button control - decommision the Matter Node
uint32_t button_time_stamp = 0;                // debouncing control
bool button_state = false;                     // false = released | true = pressed

void setup() {
  // Initialize the USER BUTTON (Boot button) that will be used to decommission the Matter Node
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.begin(115200);
  
  //Serial.setDebugOutput(true);

  // I2C for AHTx0
  Wire.begin(XIAO_SDA, XIAO_SCL);
  aht_ok = aht.begin(&Wire);
  if (!aht_ok) {
    Serial.println("AHTx0 Sensor not found on I2C. Check wiring + address.");
  }
  else {
    Serial.println("AHTx0 OK");
  }

  // E-Ink Display init
  SPI.begin(XIAO_SCK, XIAO_MISO, XIAO_MOSI, XIAO_SS); // so not the pins from pins_arduino.h are used
  init_display();

  // set initial humidity sensor measurement
  HumiditySensor.begin(0.00);
  TemperatureSensor.begin(0.00);

  // Matter beginning - Last step, after all EndPoints are initialized
  Matter.begin();

  // Check Matter Accessory Commissioning state, which may change during execution of loop()
  if (!Matter.isDeviceCommissioned()) {
    Serial.println("");
    Serial.println("Matter Node is not commissioned yet.");
    Serial.println("Initiate the device discovery in your Matter environment.");
    Serial.println("Commission it to your Matter hub with the manual pairing code or QR code");
    Serial.printf("Manual pairing code: %s\r\n", Matter.getManualPairingCode().c_str());
    Serial.printf("QR code URL: %s\r\n", Matter.getOnboardingQRCodeUrl().c_str());
    // waits for Matter Humidity and Temperature Sensor Commissioning.
    uint32_t timeCount = 0;
    while (!Matter.isDeviceCommissioned()) {
      delay(100);
      if ((timeCount++ % 50) == 0) {  // 50*100ms = 5 sec
        Serial.println("Matter Node not commissioned yet. Waiting for commissioning.");
      }
    }
    Serial.println("Matter Node is commissioned and connected to the network. Ready for use.");
  }
}

void loop() {
  const uint32_t now = millis();
  static uint32_t lastSensorReadMs = 0;
  static uint32_t lastMatterUpdateMs = 0;

  // Variables to hold the current readings
  static float current_temperature = NAN;
  static float current_humidity = NAN;

  if (now - lastSensorReadMs >= SENSOR_READ_INTERVAL_MS) {
    lastSensorReadMs = now;
    
    sensors_event_t humidityEvent, tempEvent;
    aht.getEvent(&humidityEvent, &tempEvent);
    
    current_temperature = tempEvent.temperature;
    current_humidity = humidityEvent.relative_humidity;

    // E-Ink DISPLAY LOGIC (Nested inside sensor read)
    if (isfinite(current_temperature) && isfinite(current_humidity)) {
      static float lastTemp = NAN;
      static float lastHum = NAN;
      static uint8_t changeCount = 0;

      bool isFirstRun = !isfinite(lastTemp);
      bool tempChanged = isFirstRun || (fabsf(current_temperature - lastTemp) >= TEMP_DELTA_C);
      bool humChanged  = isFirstRun || (fabsf(current_humidity - lastHum) >= HUM_DELTA_PCT);

      if (tempChanged || humChanged) {
        // Update directions
        if (!isFirstRun) {
          if (tempChanged) tempDir = (current_temperature > lastTemp) ? 1 : -1;
          if (humChanged)  humDir  = (current_humidity > lastHum) ? 1 : -1;
        }

        changeCount++;
        bool doFull = isFirstRun || (changeCount >= FULL_REFRESH_EVERY_N_CHANGES);

        if (doFull) {
          drawTHFull(current_temperature, current_humidity);
          changeCount = 0;
        } else {
          if (tempChanged) drawTempPartial(current_temperature);
          if (humChanged)  drawHumPartial(current_humidity);
        }

        lastTemp = current_temperature;
        lastHum = current_humidity;
      }
    }
  }

  // Matter Update Logic (every 5 seconds)
  if (now - lastMatterUpdateMs >= MatterUpdateIntervalMs) {
    lastMatterUpdateMs = now;
    
    if (isfinite(current_temperature) && isfinite(current_humidity)) {
      Serial.printf("Matter Update: %.02f C, %.02f%%\r\n", current_temperature, current_humidity);
      TemperatureSensor.setTemperature(current_temperature);
      HumiditySensor.setHumidity(current_humidity);
    }
  }

  
  bool currentButtonReading = (digitalRead(buttonPin) == LOW);
  // Check if the button has been pressed or released
  if (currentButtonReading && !button_state) {
    // Button just pressed
    button_time_stamp = now; // record the time while the button is pressed.
    button_state = true; // pressed.
  } else if (!currentButtonReading && button_state) {
    // Button just released
    button_state = false; // released
  }

  // Onboard User Button is kept pressed for longer than 5 seconds in order to decommission matter node
  if (button_state) {
    uint32_t time_diff = now - button_time_stamp;
    if (time_diff > decommissioningTimeoutMs) {
      Serial.println("Decommissioning TH Sensor Matter Accessory. It shall be commissioned again.");
      Matter.decommission();
      button_state = false; 
    }
  }

  /*
  // get temperature and humidity from AHTx0 sensor
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  const float current_temperature = temp.temperature;
  const float current_humidity = humidity.relative_humidity;

  static uint32_t timeCounter = 0;
  

  //E-Ink Logic
  static uint32_t lastReadMs = 0;
  static float lastTemp = NAN;
  static float lastHum = NAN;
  static uint8_t changeCount = 0;

  const uint32_t now = millis();
  if (now - lastReadMs < SENSOR_READ_INTERVAL_MS) return;
  lastReadMs = now;

   if (!isfinite(current_temperature) || !isfinite(current_humidity)) return;

  // First valid reading -> full draw
  if (!isfinite(lastTemp) || !isfinite(lastHum))
  {
    tempDir = 0;
    humDir  = 0;
    drawTHFull(current_temperature, current_humidity);
    lastTemp = current_temperature;
    lastHum = current_humidity;
    changeCount = 0;
    return;
  }

  const bool tempChanged = fabsf(current_temperature - lastTemp) >= TEMP_DELTA_C; //abs function for float
  const bool humChanged  = fabsf(current_humidity - lastHum) >= HUM_DELTA_PCT;

  if (tempChanged) tempDir = (current_temperature > lastTemp) ? 1 : -1;
  if (humChanged)  humDir  = (current_humidity > lastHum)  ? 1 : -1;

  if (!tempChanged && !humChanged) return;

  changeCount++;
  const bool doFull = (changeCount >= FULL_REFRESH_EVERY_N_CHANGES);

  if (doFull)
  {
    drawTHFull(current_temperature, current_humidity);
    changeCount = 0;
  }
  else
  {
    // fast partial updates, only redraw the area that changed
    if (tempChanged) drawTempPartial(current_temperature);
    if (humChanged)  drawHumPartial(current_humidity);
  }

  lastTemp = current_temperature;
  lastHum = current_humidity;
 
  
  //Matter Logic
  // Print the current humidity and temperature value every 5s
  if (!(timeCounter++ % 10)) {  // delaying for 500ms x 10 = 5s
    Serial.printf("Current Humidity is %.02f%%\r\n", HumiditySensor.getHumidity());
    Serial.printf("Current Temperature is %.02fC\r\n", TemperatureSensor.getTemperature());
    // Update Humidity and Temperature from the Hardware Sensor
    // Matter APP shall display the updated humidity percent
    HumiditySensor.setHumidity(current_humidity);
    TemperatureSensor.setTemperature(current_temperature);
  }

  // Check if the button has been pressed
  if (digitalRead(buttonPin) == LOW && !button_state) {
    // deals with button debouncing
    button_time_stamp = millis();  // record the time while the button is pressed.
    button_state = true;           // pressed.
  }

  if (digitalRead(buttonPin) == HIGH && button_state) {
    button_state = false;  // released
  }

  // Onboard User Button is kept pressed for longer than 5 seconds in order to decommission matter node
  uint32_t time_diff = millis() - button_time_stamp;
  if (button_state && time_diff > decommissioningTimeoutMs) {
    Serial.println("Decommissioning TH Sensor Matter Accessory. It shall be commissioned again.");
    Matter.decommission();
  }

  delay(500);
  */
}