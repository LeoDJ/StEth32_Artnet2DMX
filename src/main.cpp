#include <Arduino.h>
#include <EEPROM.h>
#include "eth.h"
#include "display.h"
#include "dmx.h"
#include "artnet.h"


config_t _config;

void saveConfig() {
    EEPROM.put(0, _config);
}

void readConfig() {
    EEPROM.get(0, _config);
    // TODO: find better way of config version mismatch handling
    if (_config.preamble != CONFIG_PREAMBLE || _config.configVersion != CONFIG_VERSION) {
        _config = {
            CONFIG_PREAMBLE,
            CONFIG_VERSION,
            true,
            {0, 0, 0, 0},
            1,
            {1, 2, 3},
        };
        saveConfig();
    }
}


void setup() {
    DEBUG.begin(115200);
    DEBUG.println("\nStEth32 Artnet2DMX\n");
    readConfig();

    // initEthernet();
    initDisplay();
    initArtnet();
}

void loop() {
    loopDisplay();
    loopArtnet();
}