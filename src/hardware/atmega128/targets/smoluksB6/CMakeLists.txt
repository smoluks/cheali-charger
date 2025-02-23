
set(SOURCE_FILES
    defaultCalibration.cpp
    HardwareConfig.h
    SmoluksB6-pins.h
    SmoluksB6.cpp
    SmoluksB6.h
)

CHEALI_HARDWARE(smoluksB6)
CHEALI_CPU(atmega128)
CHEALI_GENERIC_CHARGER(300W)

add_definitions(-DF_CPU=20000000UL)
add_definitions(-D__HXT=20000000UL) # External Crystal Clock Frequency

CHEALI_GENERATE_AVR_EXEC()
