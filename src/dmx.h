#include <stdint.h>

#define DMX_START_CODE      0x00
#define DMX_CHANNELS_MIN    24
#define DMX_CHANNELS_MAX    512

#define UART1_TX    GPIOA, LL_GPIO_PIN_9
#define UART2_TX    GPIOA, LL_GPIO_PIN_2
#define UART3_TX    GPIOB, LL_GPIO_PIN_10

// extern uint8_t dmxBufs[3][513];
void setDmxData(uint8_t output, uint8_t* buf, uint16_t size);
void initDMX();
void loopDMX();