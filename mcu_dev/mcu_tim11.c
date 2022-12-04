//
// Created by xiaotian on 2022/11/19.
//

#include "mcu_tim11.h"
#include "main.h"

void mcu_tim11_init(void) {

    /*TIM11挂载在APB2上，速度为60MHz*/
    SETBIT(RCC->APB2ENR, 1 << 18);//1：使能 TIM11 时钟
    CLRBIT(TIM11->CR1, 3 << 8);//tDTS=tCK_INT
    SETBIT(TIM11->CR1, 1 << 7);//自动重载预装载使能 (Auto-reload preload enable)
    CLRBIT(TIM11->CR1, 1 << 2);//使能后，只有计数器上溢会生成 UEV。
    CLRBIT(TIM11->CR1, 1 << 1);//使能更新中断

    //将更新溢出频率调整为1us一次，也就是60Mhz/1000*1000=60Hz，ARR=6, PSC=10即可
    TIM11->ARR = 6 - 1;//自动重载值 (Auto-reload value)
    TIM11->PSC = 10 - 1;//预分频器值 (Prescaler value)
    CLRBIT(TIM11->EGR, 1 << 0);//手动生成一次更新事件
    CLRBIT(TIM11->SR, 1 << 0);//清除更新中断标志
    SETBIT(TIM11->DIER, 1 << 0);
    //开启中断
    uint32_t prioritygroup = NVIC_GetPriorityGrouping();
    uint32_t ep = NVIC_EncodePriority(prioritygroup, 1, 1);
    NVIC_SetPriority(TIM1_TRG_COM_TIM11_IRQn, ep);
    NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);

    // SETBIT(TIM11->CR1, 1 << 0);//使能计数器
}

volatile uint32_t tim11_flag = 0;

void TIM1_TRG_COM_TIM11_IRQHandler(void) {
    /*判断更新中断标志 (Update interrupt flag)*/
    if (TIM11->SR & (1 << 0)) {
        CLRBIT(TIM11->SR, 1 << 0); //清除中断标志
        if (tim11_flag > 0)
            tim11_flag--;
    }
}

void TIM11_Delay_us(uint32_t us) {
    tim11_flag = us;
    SETBIT(TIM11->CR1, 1 << 0);//使能计数器
    while (tim11_flag);
    //如果不使用中断的时候，就关闭定时器，以免单片机频繁进入中断浪费资源
    CLRBIT(TIM11->CR1, 1 << 0);//使能计数器
}

void TIM11_Delay_ms(uint32_t ms) {
    while (ms--) {
        TIM11_Delay_us(1000);
    }
}