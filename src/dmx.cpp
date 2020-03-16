#include "dmx.h"
#include "globals.h"

#include <Arduino.h>

void MX_TIM4_Init(void);
void sendDMX();

uint8_t dmxBufs[3][513] = {{0}};
uint8_t dmxOutBuf[3][513] = {{0}};

HardwareSerial* dmxUarts[] =    {&Serial3,  &Serial2,   &Serial1    }; 
uint8_t dmxTxPins[] =           {PB10,      PA2,        PA9         };
uint8_t dmxDePins[] =           {PB1,       PA4,        PA6         };
uint8_t numDmxUarts = sizeof(dmxUarts) / sizeof(dmxUarts[0]);

void setDmxData(uint8_t output, uint8_t* buf, uint16_t size) {
    // Serial3.print('c');
    if(output >= 0 && output <= MAX_UNIVERSES) {
        if(size > DMX_CHANNELS_MAX) {
            size = DMX_CHANNELS_MAX;
        }
        memcpy(dmxBufs[output] + 1, buf, size);
        sendDMX();
        char tmp[100];
        snprintf(tmp, 100, "DMX: %02X %02X %02X %02X\n", dmxBufs[output][1], dmxBufs[output][2], dmxBufs[output][3], dmxBufs[output][4]);
        DEBUG.print(tmp);
    }
}

inline uint8_t outputEnabled(uint8_t outputId) {
    return _config.outputs.raw & (1 << outputId);
}

uint32_t lastDmxSend = 0;
void sendDMX() {
    while(millis() - lastDmxSend < 30) {}
    lastDmxSend = millis();

    uint32_t time = micros();
    // Space for break
    for(uint8_t i = 0; i < numDmxUarts; i++) {
        if(outputEnabled(i)) { 
            pinMode(dmxTxPins[i], OUTPUT);
            digitalWrite(dmxTxPins[i], HIGH); // ensure long enough mark before next packet
            delayMicroseconds(20);
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
            dmxUarts[i]->write(dmxBufs[i], 513); // TODO: dynamic dmx length
            // HAL_UART_Transmit_DMA() // TODO: probably have to use DMA
        }
    }

    // delayMicroseconds(200);
    // DEBUG.println("Send DMX took µs: " + String(micros() - time));
}

void initDMX() {
    for(uint8_t i = 0; i < numDmxUarts; i++) {
        if(outputEnabled(i)) { 
            dmxUarts[i]->begin(250000, SERIAL_8N2);
            pinMode(dmxDePins[i], OUTPUT); // TODO: comment in when other de pins are known
            digitalWrite(dmxDePins[i], HIGH);
        }
    }
    // DEBUG.print("Pin Mode: ");
    // DEBUG.println(LL_GPIO_GetPinMode(UART2_TX));
    // digitalWrite(PA2, LOW);
    // DEBUG.print("Pin Mode: ");
    // DEBUG.println(LL_GPIO_GetPinMode(UART2_TX));

    // sendDMX();

    // TODO: fix timer
    MX_TIM4_Init();
    HAL_NVIC_SetPriority(TIM4_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
    __HAL_RCC_TIM4_CLK_ENABLE();
}

// workaround until timer is fixed
uint32_t lastSend = 0;
void loopDMX() {
    if(millis() - lastSend >= 30) { // 40 Hz
        lastSend = millis();
        // sendDMX();
    } 
}

TIM_HandleTypeDef htim4;

void TIM4_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim4);
    DEBUG.print("DMX Timer: ");
    DEBUG.println(millis());
    //sendDMX();
}


void MX_TIM4_Init(void)
{
    TIM_ClockConfigTypeDef sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 7199; // 10kHz
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 199; //10kHz / 200 = 50Hz
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
    {
        Error_Handler();
    }
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
    {
        Error_Handler();
    }
    if (HAL_TIM_OC_Init(&htim4) != HAL_OK)
    {
        Error_Handler();
    }
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
    {
        Error_Handler();
    }
    sConfigOC.OCMode = TIM_OCMODE_TIMING;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_OC_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
    {
        Error_Handler();
    }

}