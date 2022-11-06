#pragma once
#include <SPI.h>
#include <Ethernet.h>

#include "globals.h"

extern uint8_t macAddr[6];
extern char macStr[13];
extern char ipStr[16];
extern EthernetClient net;

void initEthernet();
bool connectEthernet();
bool parseIpStr();