# components/GxEPD2/CMakeLists.txt
file(GLOB_RECURSE GXEPD2_SRCS
    "src/*.c"
    "src/*.cpp"
)

idf_component_register(SRCS ${GXEPD2_SRCS}
                       INCLUDE_DIRS "src"
                       REQUIRES arduino_esp32 Adafruit-GFX-Library Adafruit_BusIO)
