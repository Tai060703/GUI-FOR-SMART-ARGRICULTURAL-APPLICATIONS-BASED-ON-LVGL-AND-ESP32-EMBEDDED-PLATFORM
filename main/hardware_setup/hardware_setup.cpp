#include "hardware_setup.h"

void arduino_setup() {
    initArduino();
    Serial.begin(115200);
    EEPROM.begin(1024); // Initialize EEPROM with size 512 bytes
    Serial.println("Arduino setup done");
}


