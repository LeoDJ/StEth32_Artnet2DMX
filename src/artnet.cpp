#include "artnet.h"
#include "globals.h"
#include "dmx.h"
#include <EthernetUdp.h>
// #include "stdarg.h"
// #include <sys/unistd.h>
#include <stdarg.h>

EthernetUDP client;
uint8_t lastSequence[MAX_UNIVERSES] = {0};

// enable printf functionality, taken from https://github.com/opendata-heilbronn/modLED/blob/master/STM32/Src/uart.c
// int _write(int file, char *data, int len) {
//     if((file != STDOUT_FILENO) && (file != STDERR_FILENO)) {
//         errno = EBADF;
//         return -1;
//     }
    
//     DEBUG.write(data, len);
//     return len;
// }

// void _printf(const char* fmt, ...) {
//     char buf[100];
//     va_list args;
//     va_start(args, fmt);
//     vsnprintf(buf, sizeof(buf), fmt, args);
//     DEBUG.write(buf);
//     va_end(args);
// }

// void printHex(uint8_t* buf, uint16_t size) {
// 	_printf("       ");
// 	for(uint8_t i = 0; i < 16; i++) {
// 		_printf("%1X  ", i);
// 	}
// 	_printf("\n%04X  ", 0);
// 	for(uint16_t i = 0; i < size; i++) {
// 		_printf("%02X ", buf[i]);
// 		if(i % 16 == 15) {
// 			_printf("\n%04X  ", i+1);
// 		}
// 	}
// 	_printf("\n");
// }

// void sendArtPollReply(IPAddress ip) {
//     artPollReply_t reply = {0};
//     // set static data
//     memcpy(reply.ID, ARTNET_HEADER, sizeof(reply.ID));
//     reply.OpCode = OpCode::PollReply;
//     reply.PortNumber = ARTNET_PORT;
//     reply.VersInfo = 1;
//     reply.Oem = 0x00FF; // OemUnknown

//     uint32_t localIp = Ethernet.localIP();
//     memcpy(reply.IpAddress, &localIp, sizeof(reply.IpAddress));
//     reply.NetSwitch = 0;
//     reply.SubSwitch = 0;

//     printHex((uint8_t*) &reply, sizeof(artPollReply_t));
// }

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
                        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
                        return true;
                    }
                }
            }
            else if(pkg->opcode == OpCode::Poll) {
                // handle ArtPoll
                // sendArtPollReply(client.remoteIP());
            }
        }
    }
    return false;
}

void initArtnet() {
    client.begin(ARTNET_PORT);
    // sendArtPollReply(IPAddress(0, 0, 0, 0)); // test
}

void loopArtnet() {
    // client.findUntil(ARTNET_HEADER, /* ? */ );
    parseArtnet();
}