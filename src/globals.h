#pragma once
#include <stdint.h>

#ifndef OVERWRITE_USART1_IRQHandler // enables DMX output on UART1
#define DEBUG Serial1 // TX=PA9, RX=PA10
#else
#define DEBUG SerialUSB
#endif

#define MAX_UNIVERSES   3

#define AP_SHORT_NAME   "StEth32Artnet2DMX" // limited to 17 chars
#define AP_LONG_NAME    AP_SHORT_NAME       // limited to 63 chars


#define MENU_SERIAL     SerialUSB
#define PIN_ENCODER_A   PA5
#define PIN_ENCODER_B   PA6
#define PIN_ENCODER_BTN PA7
#define BTN_DEBOUNCE    30

#define I2C1_SCL    PB6
#define I2C1_SDA    PB7

#define DISPLAY_SCL I2C1_SCL
#define DISPLAY_SDA I2C1_SDA

#define SPI2_MOSI   PB15
#define SPI2_MISO   PB14
#define SPI2_SCK    PB13
#define SPI2_CS     PB12

#define SPI1_MOSI   PA7
#define SPI1_MISO   PA6
#define SPI1_SCK    PA5
#define SPI1_CS     PA4

#define CONFIG_PREAMBLE 0x55AA
#define CONFIG_VERSION 1
typedef struct {
    uint16_t preamble;
    uint8_t configVersion;
    bool dhcp;
    uint8_t ip[4];
    union {
        struct {
            uint8_t uart3 : 1;
            uint8_t uart2 : 1;
            uint8_t uart1 : 1;
        };
        uint8_t raw;
    } outputs;
    uint16_t universes[MAX_UNIVERSES];
} config_t;

extern config_t _config;