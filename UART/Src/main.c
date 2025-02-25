#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"


#define SOFT_IMPL   0   
#define HARD_IMPL   1
#define IMPL_MODE HARD_IMPL

#if (IMPL_MODE == HARD_IMPL)
#define UART_TX_PIN  GPIO_Pin_9
#define UART_RX_PIN  GPIO_Pin_10
#define UART_GPIO    GPIOA
#define UART        USART1

#define LED_PIN    GPIO_Pin_13
#define LED_GPIO   GPIOC

volatile uint32_t timer2_millis = 0;
uint8_t dataSend[] = "hoangdc3deptrai\n";
volatile uint8_t receivedData;

void RCC_Config(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_USART1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}

void GPIO_Config(void) {
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.GPIO_Pin = UART_TX_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(UART_GPIO, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = UART_RX_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(UART_GPIO, &GPIO_InitStruct);

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

void USART_Config(void) {
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    USART_InitStruct.USART_BaudRate = 115200;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(UART, &USART_InitStruct);

    USART_ITConfig(UART, USART_IT_RXNE, ENABLE);
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    USART_Cmd(UART, ENABLE);
}

void USART1_IRQHandler(void) {
    if (USART_GetITStatus(UART, USART_IT_RXNE) != RESET) {
        receivedData = USART_ReceiveData(UART);
        if (receivedData == '1') {
            GPIO_SetBits(LED_GPIO, LED_PIN);
        } else if (receivedData == '0') {
            GPIO_ResetBits(LED_GPIO, LED_PIN);
        }
        USART_SendData(UART, receivedData);
        while (USART_GetFlagStatus(UART, USART_FLAG_TXE) == RESET);
    }
}

void USART_SendString(uint8_t *str) {
    while (*str) {
        USART_SendData(UART, *str++);
        while (USART_GetFlagStatus(UART, USART_FLAG_TXE) == RESET);
    }
}

int main(void) {
    RCC_Config();
    GPIO_Config();
    TIM_Config();
    USART_Config();

    while (1) {
        USART_SendString(dataSend);
        delay_ms(1000);
    }
}
#else
// not impl yet !!!
#endif