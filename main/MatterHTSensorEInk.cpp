#include <Matter.h>

// List of Matter Endpoints for this Node
// Matter Humidity Sensor Endpoint
MatterTemperatureSensor SimulatedTemperatureSensor;
MatterHumiditySensor SimulatedHumiditySensor;

// set your board USER BUTTON pin here - decommissioning button
const uint8_t buttonPin = BOOT_PIN;  // Set your pin here. Using BOOT Button.

// Button control - decommision the Matter Node
uint32_t button_time_stamp = 0;                // debouncing control
bool button_state = false;                     // false = released | true = pressed
const uint32_t decommissioningTimeout = 5000;  // keep the button pressed for 5s, or longer, to decommission

// Simulate a humidity sensor - add your preferred humidity sensor library code here
float getSimulatedHumidity() {
  // The Endpoint implementation keeps an uint16_t as internal value information,
  // which stores data in 1/100th of humidity percent
  static float simulatedHumidityHWSensor = 10.0;

  // it will increase from 10% to 30% in 0.5% steps to simulate a humidity sensor
  simulatedHumidityHWSensor = simulatedHumidityHWSensor + 0.5;
  if (simulatedHumidityHWSensor > 30) {
    simulatedHumidityHWSensor = 10;
  }

  return simulatedHumidityHWSensor;
}

// Simulate a temperature sensor - add your preferred temperature sensor library code here
float getSimulatedTemperature() {
  // The Endpoint implementation keeps an int16_t as internal value information,
  // which stores data in 1/100th Celsius.
  static float simulatedTempHWSensor = -10.0;

  // it will increase from -10C to 10C in 0.5C steps to simulate a temperature sensor
  simulatedTempHWSensor = simulatedTempHWSensor + 0.5;
  if (simulatedTempHWSensor > 10) {
    simulatedTempHWSensor = -10;
  }

  return simulatedTempHWSensor;
}


void setup() {
  // Initialize the USER BUTTON (Boot button) that will be used to decommission the Matter Node
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.begin(115200);
  
  //while (!Serial) {
   // delay(100);
  //}

  //Serial.setDebugOutput(true);

  printf("hello from printf\n");
  Serial.println("Matter Humidity and Temperature Sensor Accessory Example");

  // set initial humidity sensor measurement
  // Simulated Sensor - it shall initially print 95% and then move to the 10% to 30% humidity range
  SimulatedHumiditySensor.begin(95.00);
  SimulatedTemperatureSensor.begin(-25.00);

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
    // waits for Matter Humidity Sensor Commissioning.
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

  // Print the current humidity value every 5s
  if (!(timeCounter++ % 10)) {  // delaying for 500ms x 10 = 5s
    // Print the current humidity value
    Serial.printf("Current Humidity is %.02f%%\r\n", SimulatedHumiditySensor.getHumidity());
    Serial.printf("Current Temperature is %.02fC\r\n", SimulatedTemperatureSensor.getTemperature());
    // Update Humidity from the (Simulated) Hardware Sensor
    // Matter APP shall display the updated humidity percent
    SimulatedHumiditySensor.setHumidity(getSimulatedHumidity());
    SimulatedTemperatureSensor.setTemperature(getSimulatedTemperature());
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