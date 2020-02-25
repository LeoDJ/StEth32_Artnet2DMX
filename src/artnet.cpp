#include "artnet.h"
#include "globals.h"
#include "dmx.h"
#include <EthernetUdp.h>

EthernetUDP client;
uint8_t lastSequence[MAX_UNIVERSES] = {0};

bool parseArtnet() {
    size_t pkgSize = client.parsePacket();

    if (pkgSize == 0) {
        return false;
    }

    uint8_t workBuf[pkgSize];
    client.read(workBuf, pkgSize);
    
    artnetPacket_t* pkg = (artnetPacket_t*)workBuf;

    if(strcmp(pkg->headerStr, ARTNET_HEADER)) {
        if(pkg->protocolVersion >= PROTOCOL_VER) {
            if(pkg->opcode == OpCode::Dmx) {
                for(int i = 0; i < MAX_UNIVERSES; i++) {
                    if(pkg->universe == _config.universes[i]) {
                        // if(pkg->sequence == 0 || pkg->sequence > lastSequence[i]) // TODO: sequence handling
                        setDmxData(i, pkg->data, pkg->length);
                        // handle universe
                        return true;
                    }
                }
            }
            else if(pkg->opcode == OpCode::Poll) {
                // handle ArtPoll
            }
        }
    }
    return false;

}

void initArtnet() {
    client.begin(ARTNET_PORT);
}

void loopArtnet() {
    // client.findUntil(ARTNET_HEADER, /* ? */ );
    parseArtnet();
}