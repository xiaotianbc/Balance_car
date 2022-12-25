//
// Created by xiaotian on 2022/12/21.
//

#ifndef F401_BALANCE_CAR_BOOTLOADER_BOARD_BAT_ADC_H
#define F401_BALANCE_CAR_BOOTLOADER_BOARD_BAT_ADC_H

#include "main.h"

extern __IO uint16_t uhADCxConvertedValue;
extern __IO uint32_t uwADCxConvertedVoltage;


void ADC_open(void);

__STATIC_FORCEINLINE float get_vbat_value() {
    return ((float) uhADCxConvertedValue * 21.15f) / 0xfff;
}

#endif //F401_BALANCE_CAR_BOOTLOADER_BOARD_BAT_ADC_H
