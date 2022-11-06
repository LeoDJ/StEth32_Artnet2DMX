#include "artnet.h"
#include "globals.h"
#include "dmx.h"
#include "eth.h"
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

void sendArtPollReply(IPAddress fromIp) {
    artPollReply_t reply = {0};

    // data generation takes ~110µs
    // set static data
    memcpy(reply.ID, ARTNET_HEADER, sizeof(reply.ID));
    reply.OpCode = OpCode::PollReply;
    reply.PortNumber = ARTNET_PORT;
    reply.VersInfo = 1;
    reply.Oem = 0x00FF; // OemUnknown
    reply.UbeaVersion = 0;
    reply.EstaMan = 0;
    strncpy(reply.ShortName, AP_SHORT_NAME, sizeof(reply.ShortName));
    strncpy(reply.LongName, AP_LONG_NAME, sizeof(reply.LongName));
    snprintf(reply.NodeReport, sizeof(reply.NodeReport), "OK");

    apSt1_t status1;
    status1.ubeaSupported = 0;
    status1.rdmCapable = 0;
    status1.bootMode = 0;
    status1.portAddrAuth = 0b01;    // front panel authority
    status1.indicatorCtrl = 0b11;   // normal
    reply.Status1 = status1;

    apSt2_t status2;
    status2.browserCfg = 0;
    status2.dhcpCapable = 1;
    status2.artNet3Capable = 1;     // 15-bit addresses
    reply.Status2 = status2;

    // dynamic data
    uint32_t localIp = Ethernet.localIP();
    memcpy(reply.IpAddress, &localIp, sizeof(reply.IpAddress));
    memcpy(reply.BindIp, &localIp, sizeof(reply.BindIp));
    memcpy(reply.Mac, macAddr, sizeof(reply.Mac));
    reply.Status2.ipFromDHCP = _config.dhcp;
    
    reply.NetSwitch = (_config.universes[0] & 0x7F00) >> 8;     // take base universe address from first universe in config
    reply.SubSwitch = (_config.universes[0] & 0x00F0) >> 4;

    uint8_t numOutputs = 0;
    for(int i = 0; i < MAX_UNIVERSES; i++) {
        apPT_t portType = {0};
        apGO_t goodOutput = {0};
        if(_config.outputs.raw & (1 << i)) {
            numOutputs += 1;

            portType.outputImplemented = 1;
            portType.type = PortType::DMX512;

            goodOutput.dataTransmitted = 1; // TODO: implement when implementing timouts

            reply.SwOut[i] = _config.universes[i] & 0x000F;
        }
        reply.PortTypes[i] = portType;
        reply.GoodInput[i].raw = 0;
        reply.GoodOutput[i] = goodOutput;
    }
    reply.NumPorts = numOutputs;

    // printHex((uint8_t*) &reply, sizeof(artPollReply_t));

    // sending takes ~2.3ms
    client.beginPacket(fromIp, ARTNET_PORT);
    client.write((uint8_t*)&reply, sizeof(artPollReply_t));
    client.endPacket();
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
                        digitalWrite(PIN_LED_1, !digitalRead(PIN_LED_1));
                        // _printf("%10d %d\n", micros(), pkg->universe);
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
}

void loopArtnet() {
    parseArtnet();
}