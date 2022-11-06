#include "dmx.h"
#include "globals.h"
#include "cubemx/dma.h"
#include "cubemx/usart.h"

#include <Arduino.h>

void MX_TIM4_Init(void);
void sendDMX();

HardwareTimer dmxTimer(TIM4);

uint8_t dmxBufs[3][513] = {{0}};
// uint8_t dmxOutBuf[3][513] = {{0}}; // ToDo: maybe double buffering, if necessary?

UART_HandleTypeDef* dmxUarts[] =    {&huart3,  &huart2,    &huart1 };
uint8_t dmxTxPins[] =               {PB10,      PA2,        PA9     };
uint8_t dmxDePins[] =               {PA4,       PA5,        PB1     };
uint8_t numDmxUarts = sizeof(dmxUarts) / sizeof(dmxUarts[0]);

void setDmxData(uint8_t output, uint8_t* buf, uint16_t size) {
    if(output >= 0 && output <= MAX_UNIVERSES) {
        if(size > DMX_CHANNELS_MAX) {
            size = DMX_CHANNELS_MAX;
        }
        memcpy(dmxBufs[output] + 1, buf, size);
        // char tmp[100];
        // snprintf(tmp, 100, "DMX: %02X %02X %02X %02X\n", dmxBufs[output][1], dmxBufs[output][2], dmxBufs[output][3], dmxBufs[output][4]);
        // DEBUG.print(tmp);
    }
}

inline uint8_t outputEnabled(uint8_t outputId) {
    return _config.outputs.raw & (1 << outputId);
}

void sendDMX() {
    // Space for break
    for(uint8_t i = 0; i < numDmxUarts; i++) {
        if(outputEnabled(i)) { 
            pinMode(dmxTxPins[i], OUTPUT);
            digitalWrite(dmxTxPins[i], LOW); 
            // memcpy(dmxOutBuf[i], dmxBufs[i], 513);
        }
    }
    delayMicroseconds(200); // TODO: make this nicer with timers
    
    // Mark after break
    for(uint8_t i = 0; i < numDmxUarts; i++) {
        if(outputEnabled(i)) { 
            digitalWrite(dmxTxPins[i], HIGH);
        }
    }
    delayMicroseconds(20); // already produces a suitable delay of 15us without this, but eh

    // do output with start byte
    for(uint8_t i = 0; i < numDmxUarts; i++) {
        if(outputEnabled(i)) { 
            pin_function(digitalPinToPinName(dmxTxPins[i]), STM_PIN_DATA(STM_MODE_AF_PP, GPIO_NOPULL, 0));
            HAL_UART_Transmit_DMA(dmxUarts[i], dmxBufs[i], 513);
        }
    }

    // digitalWrite(PIN_LED_1, !digitalRead(PIN_LED_1));
}

void sendDMX(HardwareTimer* ht) {
    sendDMX();
}

void initDMX() {
    for(uint8_t i = 0; i < numDmxUarts; i++) {
        if(outputEnabled(i)) { 
            pinMode(dmxDePins[i], OUTPUT);
            digitalWrite(dmxDePins[i], HIGH);
        }
    }

    MX_DMA_Init();
    #ifdef OVERWRITE_USART1_IRQHandler
        MX_USART1_UART_Init();
    #endif
    MX_USART2_UART_Init();
    MX_USART3_UART_Init();

    dmxTimer.attachInterrupt(sendDMX);
    dmxTimer.setPrescaleFactor(72); // 1MHz
    dmxTimer.setOverflow(23000);    // 1MHz / 23000us ~= 43.5Hz
    dmxTimer.resume();
}

void loopDMX() {
}