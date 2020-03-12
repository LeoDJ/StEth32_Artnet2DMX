#include "artnet.h"
#include "globals.h"
#include "dmx.h"
#include <EthernetUdp.h>
// #include "stdarg.h"
#include <sys/unistd.h>

EthernetUDP client;
uint8_t lastSequence[MAX_UNIVERSES] = {0};

// enable printf functionality, taken from https://github.com/opendata-heilbronn/modLED/blob/master/STM32/Src/uart.c
int _write(int file, char *data, int len) {
    if((file != STDOUT_FILENO) && (file != STDERR_FILENO)) {
        errno = EBADF;
        return -1;
    }
    
    DEBUG.write(data, len);
    return len;
}

// void _printf(const char* fmt, ...) {
//     va_list args;
//     char buf[100];
//     snprintf(buf, 100, fmt, args);
//     DEBUG.write(buf);
// }

void printHex(uint8_t* buf, uint16_t size) {
	printf("       ");
	for(uint8_t i = 0; i < 16; i++) {
		printf("%1X  ", i);
	}
	printf("\n%04X  ", 0);
	for(uint16_t i = 0; i < size; i++) {
		printf("%02X ", buf[i]);
		if(i % 16 == 15) {
			printf("\n%04X  ", i+1);
		}
	}
	printf("\n");
}

void sendArtPollReply(IPAddress ip) {
    artPollReply_t reply = {0};
    // set static data
    memcpy(reply.ID, ARTNET_HEADER, sizeof(reply.ID));
    reply.OpCode = OpCode::PollReply;
    reply.PortNumber = ARTNET_PORT;
    reply.VersInfo = 1;
    reply.Oem = 0x00FF; // OemUnknown

    uint32_t localIp = Ethernet.localIP();
    memcpy(reply.IpAddress, &localIp, sizeof(reply.IpAddress));
    reply.NetSwitch = 0;
    reply.SubSwitch = 0;

    printHex((uint8_t*) &reply, sizeof(artPollReply_t));
}

bool parseArtnet() {
    size_t pkgSize = client.parsePacket();

    if (pkgSize == 0) {
        return false;
    }
    uint8_t workBuf[pkgSize];
    client.read(workBuf, pkgSize);
    
    artnetPacket_t* pkg = (artnetPacket_t*)workBuf;

    if(!strcmp(pkg->headerStr, ARTNET_HEADER)) { //strcmp returns 0 if successful
        if(pkg->protocolVersion >= PROTOCOL_VER) {
            if(pkg->opcode == OpCode::Dmx) {
                for(int i = 0; i < MAX_UNIVERSES; i++) {
                    if(pkg->universe == _config.universes[i]) {
                        // if(pkg->sequence == 0 || pkg->sequence > lastSequence[i]) // TODO: sequence handling
                        setDmxData(i, pkg->data, pkg->length);
                        return true;
                    }
                }
            }
            else if(pkg->opcode == OpCode::Poll) {
                // handle ArtPoll
                sendArtPollReply(client.remoteIP());
            }
        }
    }
    return false;
}

void initArtnet() {
    client.begin(ARTNET_PORT);
    sendArtPollReply(IPAddress(0, 0, 0, 0)); // test
}

void loopArtnet() {
    // client.findUntil(ARTNET_HEADER, /* ? */ );
    parseArtnet();
}