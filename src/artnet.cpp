#include "artnet.h"
#include <EthernetUdp.h>

EthernetUDP client;

void initArtnet() {
    client.begin(ARTNET_PORT);
}

void loopArtnet() {
    //client.findUntil(ARTNET_HEADER, /* ? */ );
}