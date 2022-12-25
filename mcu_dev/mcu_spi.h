//
// Created by xiaotian on 2022/12/3.
//

#ifndef F401_BALANCE_CAR_BOOTLOADER_MCU_SPI_H
#define F401_BALANCE_CAR_BOOTLOADER_MCU_SPI_H


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


extern s32 mcu_spi_init(void);

extern s32 mcu_spi_open(SPI_DEV dev, SPI_MODE mode, u16 pre);

extern s32 mcu_spi_close(SPI_DEV dev);

extern s32 mcu_spi_transfer(SPI_DEV dev, u8 *snd, u8 *rsv, s32 len);

extern s32 mcu_spi_cs(SPI_DEV dev, u8 sta);

void mcu_spi2_send_fast_without_cs(uint8_t *txbuffer, uint8_t len);



/*
 * 除了DMA以外最快的传输函数，缺少了健壮性，
 * 把函数放在头文件里(__STATIC_FORCEINLINE没法放在源文件)，
 * 用__STATIC_FORCEINLINE修饰，效果更快
 */
__STATIC_FORCEINLINE void mcu_spi2_send_byte_ll(uint8_t b) {
    /* 忙等待 */
    /*为了节省时间 ，这里先注释掉，理论上来说，是不需要的，如果每次都在接收的话*/
    //  while ((SPI_DEVICE->SR & SPI_I2S_FLAG_BSY) != RESET);
    /* 清空SPI缓冲数据，防止读到上次传输遗留的数据 */
    /*为了节省时间 ，这里先注释掉，理论上来说，是不需要的，如果每次都在接收的话*/
//    time_out = 0;
//    while ((SPI_DEVICE->SR & SPI_I2S_FLAG_RXNE) != RESET) {
//        (void) SPI_DEVICE->DR;
//    }

#ifndef SPI_DEVICE
#define SPI_DEVICE SPI2
#endif
    /* 开始传输 */
    // 写数据
    SPI_DEVICE->DR = b;
    // 等待接收结束
    while ((SPI_DEVICE->SR & SPI_I2S_FLAG_RXNE) == RESET) {
    }
    (void) SPI_DEVICE->DR;
}

/*
 * 除了DMA以外最快的传输函数，缺少了健壮性，
 * 把函数放在头文件里(__STATIC_FORCEINLINE没法放在源文件)，
 * 用__STATIC_FORCEINLINE修饰，效果更快
 */
__STATIC_FORCEINLINE void mcu_spi2_send_16bit_ll(uint16_t b) {
    /* 忙等待 */
    /*为了节省时间 ，这里先注释掉，理论上来说，是不需要的，如果每次都在接收的话*/
    //  while ((SPI_DEVICE->SR & SPI_I2S_FLAG_BSY) != RESET);
    /* 清空SPI缓冲数据，防止读到上次传输遗留的数据 */
    /*为了节省时间 ，这里先注释掉，理论上来说，是不需要的，如果每次都在接收的话*/
//    time_out = 0;
//    while ((SPI_DEVICE->SR & SPI_I2S_FLAG_RXNE) != RESET) {
//        (void) SPI_DEVICE->DR;
//    }

#ifndef SPI_DEVICE
#define SPI_DEVICE SPI2
#endif
    /* 开始传输 */
    // 写数据
    SPI_DEVICE->DR = b;
    // 等待接收结束
    while ((SPI_DEVICE->SR & SPI_I2S_FLAG_RXNE) == RESET) {
    }
    (void) SPI_DEVICE->DR;
}

/**
 * 设置SPI2的数据格式为16bit/8bit
 */
#define  SPI2_set_dataformat_16bit() do{SPI2->CR1 |= (1 << 11);}while(0)
#define  SPI2_set_dataformat_8bit() do{SPI2->CR1 &= ~(1 << 11);}while(0)


#endif //F401_BALANCE_CAR_BOOTLOADER_MCU_SPI_H
