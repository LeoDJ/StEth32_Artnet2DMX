#include "dmx.h"
#include "globals.h"

#include <Arduino.h>

uint8_t dmxBufs[3][513] = {{0}};

void setDmxData(uint8_t output, uint8_t* buf, uint16_t size) {
    if(output >= 0 && output <= MAX_UNIVERSES) {
        if(size > DMX_CHANNELS_MAX) {
            size = DMX_CHANNELS_MAX;
        }
        memcpy(dmxBufs[output], buf, size);
    }
}


void initDMX() {
    Serial2.begin(250000, SERIAL_8N2);
    
}