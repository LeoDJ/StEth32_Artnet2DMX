#include <Arduino.h>
#include <EEPROM.h>
#include "eth.h"
#include "display.h"
#include "dmx.h"
#include "artnet.h"
#include "dmx.h"


config_t _config;

void saveConfig() {
    EEPROM.put(0, _config);
}

void readConfig() {
    EEPROM.get(0, _config);
    // TODO: find better way of config version mismatch handling
    if (_config.preamble != CONFIG_PREAMBLE || _config.configVersion != CONFIG_VERSION) {
        // initial values when no config exists
        _config = {
            CONFIG_PREAMBLE,
            CONFIG_VERSION,
            true,               // DHCP
            {0, 0, 0, 0},       // IP
            {1, 1, 1},          // Outputs
            {1, 2, 3},          // Universe-Output Mapping, only least 4 bits are allowed to change
        };
        saveConfig();
    }
}


void setup() {
    DEBUG.begin(115200);
    DEBUG.println("\nStEth32 Artnet2DMX\n");
    readConfig();

    pinMode(PIN_LED_1, OUTPUT);
    digitalWrite(PIN_LED_1, HIGH); // active low
    pinMode(PIN_LED_2, OUTPUT);
    digitalWrite(PIN_LED_2, HIGH); // active low

    initDisplay();
    loopDisplay(); // show initial display content
    initEthernet();
    connectEthernet();
    initArtnet();
    initDMX();
}

void loop() {
    loopDisplay();
    loopArtnet();
    loopDMX();
}