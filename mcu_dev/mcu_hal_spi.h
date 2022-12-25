//
// Created by xiaotian on 2022/12/24.
//

#ifndef F401_BALANCE_CAR_BOOTLOADER_MCU_HAL_SPI_H
#define F401_BALANCE_CAR_BOOTLOADER_MCU_HAL_SPI_H

#include "stm32f4xx.h"

typedef enum {
    DEV_SPI_NULL = 0,
    DEV_SPI_2_1 = 0x21,        //SPI_2_1- LCD屏幕
    DEV_SPI_2_2 = 0X22,//NRF24L01接口
    DEV_SPI_3_2,    //底板板上的SPI使用SPI3，定义为SPI_3_2
    DEV_SPI_3_3,        //外扩的SPI定义为SPI_3_3
} SPI_DEV;


typedef enum {
    SPI_MODE_0 = 0,
    SPI_MODE_1,
    SPI_MODE_2,
    SPI_MODE_3,
    SPI_MODE_MAX
} SPI_MODE;

typedef struct {
    SPI_DEV DevX;
    SPI_MODE mode;
    SPI_TypeDef* _SPIx;

} spi_handle;


#endif //F401_BALANCE_CAR_BOOTLOADER_MCU_HAL_SPI_H
