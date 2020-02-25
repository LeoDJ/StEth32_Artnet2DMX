#pragma once
#include <SPI.h>
#include <Ethernet.h>

#include "globals.h"

extern char macStr[13];
extern EthernetClient net;

void initEthernet();
bool connectEthernet();