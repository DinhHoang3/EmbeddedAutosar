#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_i2c.h"

#define SOFT_IMPL   0   
#define HARD_IMPL   1
#define IMPL_MODE HARD_IMPL

#if (IMPL_MODE == HARD_IMPL)
#define I2C_SCL    GPIO_Pin_6
#define I2C_SDA    GPIO_Pin_7
#define I2C_GPIO   GPIOB

#define LED_PIN    GPIO_Pin_13
#define LED_GPIO   GPIOC

volatile uint32_t timer2_millis = 0;
uint8_t dataSend[] = "hoangdc3@fpt.com\n";

void RCC_Config(void);
void GPIO_Config(void);
void TIM_Config(void);
void I2C_Config(void);
void delay_ms(uint32_t ms);
void I2C_Send1Byte(uint8_t address, uint8_t data);
uint8_t I2C_Receive1Byte(uint8_t address);
void I2C_SendString(uint8_t address, uint8_t *data, uint8_t length);

int main(void) {
    RCC_Config();
    GPIO_Config();
    TIM_Config();
    I2C_Config();
    uint8_t receivedData;

    while (1) {
        I2C_Send1Byte(0x50, 0xAA);
        receivedData = I2C_Receive1Byte(0x50);

        if (receivedData == 0xAC) {
            GPIOC->ODR ^= GPIO_Pin_13;
        }
        delay_ms(1000);
    }
}

void RCC_Config(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}

void GPIO_Config(void) {
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.GPIO_Pin = I2C_SCL | I2C_SDA;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(I2C_GPIO, &GPIO_InitStruct);

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

void I2C_Config(void) {
    I2C_InitTypeDef I2C_InitStruct;

    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStruct.I2C_ClockSpeed = 100000;

    I2C_Init(I2C1, &I2C_InitStruct);
    I2C_Cmd(I2C1, ENABLE);
}

void I2C_Send1Byte(uint8_t address, uint8_t data) {
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C1, address, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    I2C_SendData(I2C1, data);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_GenerateSTOP(I2C1, ENABLE);
}

uint8_t I2C_Receive1Byte(uint8_t address) {
    uint8_t data;
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C1, address, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    data = I2C_ReceiveData(I2C1);
    I2C_GenerateSTOP(I2C1, ENABLE);
    return data;
}

void I2C_SendString(uint8_t address, uint8_t *data, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        I2C_Send1Byte(address, data[i]);
        delay_ms(10);
    }
}
#else
// not impl yet !!!
#endif
