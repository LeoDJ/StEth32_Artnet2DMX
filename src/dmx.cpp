#include "dmx.h"

#include <Arduino.h>

uint8_t dmxBufs[3][513] = {{0}};


void initDMX() {
    Serial2.begin(250000, SERIAL_8N2);
    
}