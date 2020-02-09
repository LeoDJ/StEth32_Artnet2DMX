#include <stdint.h>

#define DEBUG Serial1 // TX=PA9, RX=PA10

#define CONFIG_PREAMBLE 0x55AA
#define CONFIG_VERSION 1
typedef struct {
    uint16_t preamble;
    uint8_t configVersion;
    bool dhcp;
    uint8_t ip[4];
    uint8_t numDmxOutputs;
    uint16_t universes[3];
} config_t;

extern config_t _config;

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