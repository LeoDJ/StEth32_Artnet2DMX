

#define DEBUG Serial1

#define CONFIG_PREAMBLE 0x55AA
#define CONFIG_VERSION 1
typedef struct {
    uint16_t preamble;
    uint8_t configVersion;
    bool dhcp;
    uint8_t ip[4];
    uint8_t numDmxOutputs;
    uint16_t universes[4];
} config_t;

extern config_t config;