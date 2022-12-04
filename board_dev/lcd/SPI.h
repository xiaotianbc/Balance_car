#include "main.h"

#ifndef _SPI_H_
#define _SPI_H_

//本测试程序使用的是硬件SPI接口驱动
//除了SPI时钟信号以及SPI读、写信号引脚不可更改，其他引脚都可以更改
//SPI的时钟引脚定义固定为PB3
//SPI的读数据引脚定义固定为PB4
//SPI的写数据引脚定义固定为PB5
 

void SPI_SetSpeed(SPI_TypeDef* SPIx,u8 SpeedSet);


void SPI2_Send_framebuffer_init_to_16bit(uint8_t is16bit);



#endif
