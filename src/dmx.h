#include <stdint.h>

#define DMX_START_CODE      0x00
#define DMX_CHANNELS_MIN    24
#define DMX_CHANNELS_MAX    512

// extern uint8_t dmxBufs[3][513];
void setDmxData(uint8_t output, uint8_t* buf, uint16_t size);