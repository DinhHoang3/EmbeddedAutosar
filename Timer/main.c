#include "stm32f10x.h"                  
#include "stm32f10x_rcc.h"              
#include "stm32f10x_gpio.h"             
#include "stm32f10x_tim.h"              

volatile uint32_t timer2_millis = 0;

void GPIO_Config(void);
void TIM2_Config(void);
void RCC_Config(void);
void delay_ms(uint32_t ms);

int main(void)
{
    RCC_Config();
    GPIO_Config();
    TIM2_Config();

    while(1)
    {
        GPIOC->ODR ^= GPIO_Pin_13;  // Đảo trạng thái LED
        delay_ms(500);              // Chờ 500ms
    }
}

void RCC_Config(void) 
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
}

void GPIO_Config(void) 
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void TIM2_Config(void) 
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    TIM_TimeBaseStruct.TIM_Prescaler = 7200 - 1;   // 72MHz / 7200 = 10kHz (0.1ms/tick)
    TIM_TimeBaseStruct.TIM_Period = 10 - 1;        // 10 ticks = 1ms
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

void TIM2_IRQHandler(void) 
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        timer2_millis++;  
    }
}

void delay_ms(uint32_t ms)
{
    uint32_t start_time = timer2_millis;
    while ((timer2_millis - start_time) < ms);
}
