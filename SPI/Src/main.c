#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_spi.h"


#define SOFT_IMPL   0   
#define HARD_IMPL   1
#define IMPL_MODE HARD_IMPL

#if (IMPL_MODE == HARD_IMPL)
#define SPI1_NSS   GPIO_Pin_4
#define SPI1_SCK   GPIO_Pin_5
#define SPI1_MISO  GPIO_Pin_6
#define SPI1_MOSI  GPIO_Pin_7
#define SPI1_GPIO  GPIOA

#define LED_PIN    GPIO_Pin_13
#define LED_GPIO   GPIOC

volatile uint32_t timer2_millis = 0;
uint8_t dataSend[] = "hoangdc3@fpt.com\n";

void RCC_Config(void);
void GPIO_Config(void);
void TIM_Config(void);
void SPI_Config(void);
void delay_ms(uint32_t ms);
void SPI_Send1Byte(uint8_t data);
uint8_t SPI_Receive1Byte(void);
void SPI_SendString(uint8_t *data, uint8_t length);

int main(void) {
    RCC_Config();
    GPIO_Config();
    TIM_Config();
    SPI_Config();
    uint8_t receivedData;

    while (1) {
        SPI_Send1Byte(0xAA);  
        receivedData  = 0;
        receivedData = SPI_Receive1Byte();

        if (receivedData == 0xAC) {
            GPIOC->ODR ^= GPIO_Pin_13;
        }
        delay_ms(1000);
    }
}

void RCC_Config(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}

void GPIO_Config(void) {
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.GPIO_Pin = SPI1_SCK | SPI1_MOSI;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SPI1_GPIO, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = SPI1_MISO;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(SPI1_GPIO, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = SPI1_NSS;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SPI1_GPIO, &GPIO_InitStruct);
    GPIO_SetBits(SPI1_GPIO, SPI1_NSS);

    GPIO_InitStruct.GPIO_Pin = LED_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_GPIO, &GPIO_InitStruct);
}

void TIM_Config(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    TIM_TimeBaseStruct.TIM_Prescaler = 7200 - 1;
    TIM_TimeBaseStruct.TIM_Period = 10 - 1;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStruct);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    TIM_Cmd(TIM2, ENABLE);
}

void TIM2_IRQHandler(void) {
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        timer2_millis++;
    }
}

void delay_ms(uint32_t ms) {
    uint32_t start_time = timer2_millis;
    while ((timer2_millis - start_time) < ms);
}

void SPI_Config(void) {
    SPI_InitTypeDef SPI_InitStruct;

    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStruct.SPI_CRCPolynomial = 7;

    SPI_Init(SPI1, &SPI_InitStruct);
    SPI_Cmd(SPI1, ENABLE);
}

void SPI_Send1Byte(uint8_t data) {
    GPIO_ResetBits(SPI1_GPIO, SPI1_NSS);
    
    while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));
    SPI_I2S_SendData(SPI1, data);

    while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
    SPI_I2S_ReceiveData(SPI1);
    
    GPIO_SetBits(SPI1_GPIO, SPI1_NSS);
}

uint8_t SPI_Receive1Byte(void) {
    GPIO_ResetBits(SPI1_GPIO, SPI1_NSS);

    while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));
    SPI_I2S_SendData(SPI1, 0xFF);

    while (!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
    uint8_t receivedData = SPI_I2S_ReceiveData(SPI1);

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));

    GPIO_SetBits(SPI1_GPIO, SPI1_NSS);
    return receivedData;
}

void SPI_SendString(uint8_t *data, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        SPI_Send1Byte(data[i]);
        delay_ms(10);
    }
}
#else
// not impl yet !!!
#endif
// PA5  (SCK)       | ---------> |  D13  (SCK) 
// PA6  (MISO)      | <--------- |  D12  (MISO)
// PA7  (MOSI)      | ---------> |  D11  (MOSI)
// PA4  (NSS/SS)    | ---------> |  D10  (SS)  
// GND              | ---------- |  GND 