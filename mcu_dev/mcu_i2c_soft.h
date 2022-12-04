#ifndef __MCU_I2C_H__
#define __MCU_I2C_H__


#include "stm32f4xx.h"

#define MCU_I2C_MODE_W 0
#define MCU_I2C_MODE_R 1


extern s32 mcu_i2c_init(void);
extern s32 mcu_i2c_transfer(u8 addr, u8 rw, u8* data, s32 datalen);
int32_t mcu_i2c_mem_read(u8 addr, uint8_t reg_addr, u8 *data, s32 datalen);
int32_t mcu_i2c_mem_write(u8 addr, uint8_t reg_addr, u8 *data, s32 datalen);
#endif
