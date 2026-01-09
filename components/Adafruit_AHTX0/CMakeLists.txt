idf_component_register(
    SRCS "Adafruit_AHTX0.cpp"
    INCLUDE_DIRS "."
    REQUIRES arduino-esp32 Adafruit_Sensor Adafruit_BusIO
)