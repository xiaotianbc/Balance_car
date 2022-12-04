//
// Created by xiaotian on 2022/11/19.
//

#ifndef F401_BALANCE_CAR_BOOTLOADER_MCU_TIM11_H
#define F401_BALANCE_CAR_BOOTLOADER_MCU_TIM11_H

#include <stdint.h>

void mcu_tim11_init(void);
void TIM11_Delay_ms(uint32_t ms);
void TIM11_Delay_us(uint32_t us);
#endif //F401_BALANCE_CAR_BOOTLOADER_MCU_TIM11_H
