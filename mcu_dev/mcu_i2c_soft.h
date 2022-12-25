#ifndef __MCU_I2C_H__
#define __MCU_I2C_H__


#include "stm32f4xx.h"

#define MCU_I2C_MODE_W 0
#define MCU_I2C_MODE_R 1


extern s32 mcu_i2c_init(void);

extern s32 mcu_i2c_transfer(u8 addr, u8 rw, u8 *data, s32 datalen);

int32_t mcu_i2c_mem_read(u8 addr, uint8_t reg_addr, u8 *data, s32 datalen);

int32_t mcu_i2c_mem_write(u8 addr, uint8_t reg_addr, u8 *data, s32 datalen);

//甚至可以兼容STM32 hal库的i2c函数
#define HAL_I2C_Mem_Read(a, b, c, d, e, f, g) mcu_i2c_mem_read(b, c, e, f)
#define HAL_I2C_Mem_Write(a, b, c, d, e, f, g) mcu_i2c_mem_write(b, c, e, f)

#endif
