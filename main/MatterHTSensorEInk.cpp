#include <Arduino.h>
#include <Matter.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>

// List of Matter Endpoints for this Node
// Matter Humidity and Temperature Sensor Endpoint
MatterTemperatureSensor TemperatureSensor;
MatterHumiditySensor HumiditySensor;

// set your board USER BUTTON pin here - decommissioning button
const uint8_t buttonPin = BOOT_PIN;  // Set your pin here. Using BOOT Button.

// I2C pins for the AHTx0 sensor
static const u_int8_t I2C_SDA_PIN = GPIO_NUM_22; //D4
static const u_int8_t I2C_SCL_PIN = GPIO_NUM_23; //D5

Adafruit_AHTX0 aht;
static bool aht_ok = false;

// Button control - decommision the Matter Node
uint32_t button_time_stamp = 0;                // debouncing control
bool button_state = false;                     // false = released | true = pressed
const uint32_t decommissioningTimeout = 5000;  // keep the button pressed for 5s, or longer, to decommission

void setup() {
  // Initialize the USER BUTTON (Boot button) that will be used to decommission the Matter Node
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.begin(115200);
  
  //Serial.setDebugOutput(true);

  // I2C for AHTx0
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  aht_ok = aht.begin(&Wire);
  if (!aht_ok) {
    Serial.println("AHTx0 Sensor not found on I2C. Check wiring + address.");
  }
  else {
    Serial.println("AHTx0 OK");
  }

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
  static uint32_t timeCounter = 0;

  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  const float current_temperature = temp.temperature;
  const float current_humidity = humidity.relative_humidity;

  // Print the current humidity value every 5s
  if (!(timeCounter++ % 10)) {  // delaying for 500ms x 10 = 5s
    // Print the current humidity and temperature value
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
  if (button_state && time_diff > decommissioningTimeout) {
    Serial.println("Decommissioning TH Sensor Matter Accessory. It shall be commissioned again.");
    Matter.decommission();
  }

  delay(500);
}