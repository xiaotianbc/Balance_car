//
// Created by xiaotian on 2022/12/3.
//

#include <stddef.h>
#include "mcu_spi.h"
#include "main.h"

#include "stm32f4xx.h"
#include "mcu_spi.h"

//#define MCU_SPI_DEBUG

#ifdef MCU_SPI_DEBUG
#define SPI_DEBUG	wjq_log
#else
#define SPI_DEBUG(a, args...)  SEGGER_RTT_printf(0, ##args)
#endif


#define MCU_SPI_WAIT_TIMEOUT 0x40000
/*
	硬件SPI使用控制器SPI2
*/
#define SPI_DEVICE SPI2
/*
	相位配置，一共四种模式
*/
typedef struct {
    u16 CPOL;
    u16 CPHA;
} _strSpiModeSet;

const _strSpiModeSet SpiModeSet[SPI_MODE_MAX] =
        {
                {SPI_CPOL_Low,  SPI_CPHA_1Edge},
                {SPI_CPOL_Low,  SPI_CPHA_2Edge},
                {SPI_CPOL_High, SPI_CPHA_1Edge},
                {SPI_CPOL_High, SPI_CPHA_2Edge}
        };

/*SPI控制设备符，每个硬件控制器必须定义一个，此处用了SPI3一个硬件SPI控制器*/
s32 DevSpi3Gd = -2;

/**
 *@brief:      mcu_spi_init
 *@details:    初始化SPI控制器，并初始化所有CS脚
 *@param[in]   void
 *@param[out]  无
 *@retval:
 */
s32 mcu_spi_init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef SPI_InitStruct;

    //SPI2-> PB15 MOSI,PB14 MISO, PB13 SCK, PB12 NRF24CS, PB9 LCD-CS

    //初始化片选，
    //PB12 NRF24CS
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_12);

    //PB9 LCD-CS
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_9);


    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;//---PB13~15
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//---复用功能
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//---推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//---100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//---上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);//---初始化

    //配置引脚复用映射
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2); //PB3 复用为 SPI3
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2); //PB4 复用为 SPI3
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2); //PB5 复用为 SPI3

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);// ---使能 SPI2 时钟
    // 复位SPI模块
    SPI_I2S_DeInit(SPI_DEVICE);

    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//---双线双向全双工
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;//---主模式
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;//---8bit帧结构
    SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;//----串行同步时钟的空闲状态为低电平
    SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;//---数据捕获于第1个时钟沿
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft; //---SPI_NSS_Hard; 片选由硬件管理，SPI控制器不管理
    SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;  //---预分频
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;//---数据传输从 MSB 位开始
    SPI_InitStruct.SPI_CRCPolynomial = 7;//---CRC 值计算的多项式

    SPI_Init(SPI_DEVICE, &SPI_InitStruct);

    SPI_Cmd(SPI_DEVICE, ENABLE);

    DevSpi3Gd = -1;
    return 0;
}

/**
 *@brief:      mcu_spi_open
 *@details:       打开SPI
 *@param[in]   SPI_DEV dev  ：SPI号
               u8 mode      模式
               u16 pre      预分频
 *@param[out]  无
 *@retval:
 */
s32 mcu_spi_open(SPI_DEV dev, SPI_MODE mode, u16 pre) {
    SPI_InitTypeDef SPI_InitStruct;

    if (DevSpi3Gd != -1)
        return -1;

    if (mode >= SPI_MODE_MAX)
        return -1;

    SPI_I2S_DeInit(SPI_DEVICE);

    SPI_Cmd(SPI1, DISABLE);

    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//---双线双向全双工
    SPI_InitStruct.SPI_Mode = SPI_Mode_Master;//---主模式
    SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b;//---8bit帧结构
    SPI_InitStruct.SPI_CPOL = SpiModeSet[mode].CPOL;
    SPI_InitStruct.SPI_CPHA = SpiModeSet[mode].CPHA;
    SPI_InitStruct.SPI_NSS = SPI_NSS_Soft; //---SPI_NSS_Hard; 片选由硬件管理，SPI控制器不管理
    SPI_InitStruct.SPI_BaudRatePrescaler = pre;  //---预分频
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;//---数据传输从 MSB 位开始
    SPI_InitStruct.SPI_CRCPolynomial = 7;//---CRC 值计算的多项式

    SPI_Init(SPI_DEVICE, &SPI_InitStruct);

    /*
        要先使能SPI，再使能CS
    */
    SPI_Cmd(SPI_DEVICE, ENABLE);

    if (dev == DEV_SPI_2_2) {
        GPIO_ResetBits(GPIOB, GPIO_Pin_14);
    } else if (dev == DEV_SPI_3_2) {
        GPIO_ResetBits(GPIOG, GPIO_Pin_15);
    } else if (dev == DEV_SPI_3_3) {
        GPIO_ResetBits(GPIOG, GPIO_Pin_6);
    } else if (dev == DEV_SPI_2_1) {  //2-1 LCD屏幕
        GPIO_ResetBits(GPIOB, GPIO_Pin_9);
    } else {
        return -1;
    }

    DevSpi3Gd = dev;


    return 0;
}

/**
 *@brief:      mcu_spi_close
 *@details:    关闭SPI 控制器
 *@param[in]   void
 *@param[out]  无
 *@retval:
 */
s32 mcu_spi_close(SPI_DEV dev) {
    if (DevSpi3Gd != dev) {
        SPI_DEBUG(LOG_DEBUG, "spi dev err\r\n");
        return -1;
    }

    if (dev == DEV_SPI_2_2) {
        GPIO_SetBits(GPIOB, GPIO_Pin_14);
    } else if (dev == DEV_SPI_3_2) {
        GPIO_SetBits(GPIOG, GPIO_Pin_15);
    } else if (dev == DEV_SPI_3_3) {
        GPIO_SetBits(GPIOG, GPIO_Pin_6);
    } else {
        return -1;
    }

    SPI_Cmd(SPI_DEVICE, DISABLE);
    DevSpi3Gd = -1;
    return 0;
}

/**
 *@brief:      mcu_spi_transfer
 *@details:    SPI 传输
 *@param[in]   u8 *snd
               u8 *rsv
               s32 len
 *@param[out]  无
 *@retval:
 */
s32 mcu_spi_transfer(SPI_DEV dev, u8 *snd, u8 *rsv, s32 len) {
    s32 i = 0;
    s32 pos = 0;
    u32 time_out = 0;
    u16 ch;

    if (dev != DevSpi3Gd) {
        SPI_DEBUG(LOG_DEBUG, "spi dev err\r\n");
        return -1;
    }

    if (((snd == NULL) && (rsv == NULL)) || (len < 0)) {
        return -1;
    }

    /* 忙等待 */
    time_out = 0;
    while (SPI_I2S_GetFlagStatus(SPI_DEVICE, SPI_I2S_FLAG_BSY) == SET) {
        if (time_out++ > MCU_SPI_WAIT_TIMEOUT) {
            return (-1);
        }
    }

    /* 清空SPI缓冲数据，防止读到上次传输遗留的数据 */
    time_out = 0;
    while (SPI_I2S_GetFlagStatus(SPI_DEVICE, SPI_I2S_FLAG_RXNE) == SET) {
        SPI_I2S_ReceiveData(SPI_DEVICE);
        if (time_out++ > 2) {
            return (-1);
        }
    }

    /* 开始传输 */
    for (i = 0; i < len;) {
        // 写数据
        if (snd == NULL)/*发送指针为NULL，说明仅仅是读数据 */
        {
            SPI_I2S_SendData(SPI_DEVICE, 0xff);
        } else {
            ch = (u16) snd[i];
            SPI_I2S_SendData(SPI_DEVICE, ch);
        }
        i++;

        // 等待接收结束
        time_out = 0;
        while (SPI_I2S_GetFlagStatus(SPI_DEVICE, SPI_I2S_FLAG_RXNE) == RESET) {
            time_out++;
            if (time_out > MCU_SPI_WAIT_TIMEOUT) {
                return -1;
            }
        }
        // 读数据
        if (rsv == NULL)/* 接收指针为空，读数据后丢弃 */
        {
            SPI_I2S_ReceiveData(SPI_DEVICE);
        } else {
            ch = SPI_I2S_ReceiveData(SPI_DEVICE);
            rsv[pos] = (u8) ch;
        }
        pos++;

    }

    return i;
}

/**
 *@brief:      mcu_spi_cs
 *@details:    操控对应SPI的CS
 *@param[in]   SPI_DEV dev
               u8 sta
 *@param[out]  无
 *@retval:
 */
s32 mcu_spi_cs(SPI_DEV dev, u8 sta) {
    if (DevSpi3Gd != dev) {
        SPI_DEBUG(0, "spi dev err\r\n");
        return -1;
    }

    if (sta == 1) {
        switch (dev) {
            case DEV_SPI_2_2:
                GPIO_SetBits(GPIOB, GPIO_Pin_14);
                break;
            case DEV_SPI_3_2:
                GPIO_SetBits(GPIOG, GPIO_Pin_15);
                break;
            case DEV_SPI_3_3:
                GPIO_SetBits(GPIOG, GPIO_Pin_6);
                break;
            default:
                break;
        }
    } else {
        switch (dev) {
            case DEV_SPI_2_2:
                GPIO_ResetBits(GPIOB, GPIO_Pin_14);
                break;
            case DEV_SPI_3_2:
                GPIO_ResetBits(GPIOG, GPIO_Pin_15);
                break;
            case DEV_SPI_3_3:
                GPIO_ResetBits(GPIOG, GPIO_Pin_6);
                break;
            default:
                break;
        }
    }

    return 0;
}


void mcu_spi2_transfer_fast_without_cs(uint8_t *txbuffer, uint8_t *rxbuffer, uint8_t len) {
    s32 i = 0;
    s32 pos = 0;
    u32 time_out = 0;
    u16 ch;

    if (((txbuffer == NULL) && (rxbuffer == NULL)) || (len < 0)) {
        return;
    }

    /* 忙等待 */
    time_out = 0;
    while (SPI_I2S_GetFlagStatus(SPI_DEVICE, SPI_I2S_FLAG_BSY) == SET) {
        if (time_out++ > MCU_SPI_WAIT_TIMEOUT) {
            return;
        }
    }

    /* 清空SPI缓冲数据，防止读到上次传输遗留的数据 */
    time_out = 0;
    while (SPI_I2S_GetFlagStatus(SPI_DEVICE, SPI_I2S_FLAG_RXNE) == SET) {
        SPI_I2S_ReceiveData(SPI_DEVICE);
        if (time_out++ > 2) {
            return;
        }
    }

    /* 开始传输 */
    for (i = 0; i < len;) {
        // 写数据

        if (txbuffer == NULL)/*发送指针为NULL，说明仅仅是读数据 */
        {
            SPI_I2S_SendData(SPI_DEVICE, 0xff);
        } else {
            ch = (u16) txbuffer[i];
            SPI_I2S_SendData(SPI_DEVICE, ch);
        }
        i++;

        // 等待接收结束
        time_out = 0;
        while (SPI_I2S_GetFlagStatus(SPI_DEVICE, SPI_I2S_FLAG_RXNE) == RESET) {
            time_out++;
            if (time_out > MCU_SPI_WAIT_TIMEOUT) {
                return;
            }
        }
        // 读数据
        if (rxbuffer == NULL)/* 接收指针为空，读数据后丢弃 */
        {
            SPI_I2S_ReceiveData(SPI_DEVICE);
        } else {
            ch = SPI_I2S_ReceiveData(SPI_DEVICE);
            rxbuffer[pos] = (u8) ch;
        }
        pos++;
    }
}


void mcu_spi2_send_fast_without_cs(uint8_t *txbuffer, uint8_t len) {
    s32 i = 0;
    u32 time_out = 0;
    /* 忙等待 */
    time_out = 0;
    while (SPI_I2S_GetFlagStatus(SPI_DEVICE, SPI_I2S_FLAG_BSY) == SET) {
        if (time_out++ > MCU_SPI_WAIT_TIMEOUT) {
            return;
        }
    }
    /* 清空SPI缓冲数据，防止读到上次传输遗留的数据 */
    time_out = 0;
    while (SPI_I2S_GetFlagStatus(SPI_DEVICE, SPI_I2S_FLAG_RXNE) == SET) {
        SPI_I2S_ReceiveData(SPI_DEVICE);
        if (time_out++ > 2) {
            return;
        }
    }
    /* 开始传输 */
    for (i = 0; i < len;) {
        // 写数据
        SPI_I2S_SendData(SPI_DEVICE, txbuffer[i]);
        i++;
        // 等待接收结束
        time_out = 0;
        while (SPI_I2S_GetFlagStatus(SPI_DEVICE, SPI_I2S_FLAG_RXNE) == RESET) {
            time_out++;
            if (time_out > MCU_SPI_WAIT_TIMEOUT) {
                return;
            }
        }
        SPI_I2S_ReceiveData(SPI_DEVICE);
    }
}
