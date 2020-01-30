#include <Arduino.h>
#include <EEPROM.h>
#include "eth.h"


config_t config;

void saveConfig() {
    EEPROM.put(0, config);
}

void readConfig() {
    EEPROM.get(0, config);
    // TODO: find better way of config version mismatch handling
    if (config.preamble != CONFIG_PREAMBLE || config.configVersion != CONFIG_VERSION) {
        config = {
            CONFIG_PREAMBLE,
            CONFIG_VERSION,
            true,
            {0, 0, 0, 0},
            1,
            {0, 0, 0, 0},
        };
        saveConfig();
    }
}


void setup() {
    DEBUG.begin(115200);
    DEBUG.println("\nStEth32 Artnet2DMX\n");
    readConfig();

    initEthernet();
}

void loop() {
    // put your main code here, to run repeatedly:
}