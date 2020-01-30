#include <Arduino.h>

#include "eth.h"

void setup() {
    DEBUG.begin(115200);
    DEBUG.println("\nStEth32 Artnet2DMX\n");

    initEthernet();

}

void loop() {
    // put your main code here, to run repeatedly:
}