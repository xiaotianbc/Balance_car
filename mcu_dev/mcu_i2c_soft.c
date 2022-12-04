/**
 * mcu 软件模拟i2c协议
 * 作者：xiaotian
 */
#include "main.h"
#include "stm32f4xx.h"
#include "mcu_i2c_soft.h"

#define  a_w(addr) ((addr<<1)&0xfe)
#define  a_r(addr) ((addr<<1)|0x01)


#define MCU_I2C_PORT GPIOB
#define MCU_I2C_SCL GPIO_Pin_10
#define MCU_I2C_SCL_pinx (10)
#define MCU_I2C_SDA GPIO_Pin_3
#define MCU_I2C_SDA_pinx (3)

#define MCU_I2C_TIMEOUT 250

/**
 *@brief:      mcu_i2c_delay
 *@details:    I2C信号延时函数
 *@param[in]   void  
 *@param[out]  无
 *@retval:     static
 */
static void mcu_i2c_delay(void) {
    volatile uint8_t i = 5;
    while (i--) {
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
    }

}

/**
 *@brief:      mcu_i2c_sda_input
 *@details:    将I2C sda IO设置为输入
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
void mcu_i2c_sda_input(void) {
#if 1
    MCU_I2C_PORT->MODER &= ~(0x03 << (2 * MCU_I2C_SDA_pinx));  //清空MODER，为input模式
#else
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = MCU_I2C_SDA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//输入模式  
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(MCU_I2C_PORT, &GPIO_InitStructure);//初始化
#endif
}

/**
 *@brief:      mcu_i2c_sda_output
 *@details:       将I2C sda IO设置为输出
 *@param[in]  void  
 *@param[out]  无
 *@retval:     
 */
void mcu_i2c_sda_output(void) {
#if 1
    MCU_I2C_PORT->MODER &= ~(0x03 << (2 * MCU_I2C_SDA_pinx));  //清空MODER
    MCU_I2C_PORT->MODER |= (0x01 << (2 * MCU_I2C_SDA_pinx));  //设置为General purpose output mode
#else
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = MCU_I2C_SDA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式   
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(MCU_I2C_PORT, &GPIO_InitStructure);//初始化
#endif
}

/**
 *@brief:      mcu_i2c_readsda
 *@details:    读SDA数据
 *@param[in]   void  
 *@param[out]  无
 *@retval:     static
 */
static s32 mcu_i2c_readsda(void) {
    if (Bit_SET == GPIO_ReadInputDataBit(MCU_I2C_PORT, MCU_I2C_SDA))
        return 1;
    else
        return 0;
}


#define  mcu_i2c_sda(sta) MCU_I2C_PORT->BSRR=(sta!=0)?MCU_I2C_SDA:(MCU_I2C_SDA<<16)
#define  mcu_i2c_scl(sta) MCU_I2C_PORT->BSRR=(sta!=0)?MCU_I2C_SCL:(MCU_I2C_SCL<<16)

/**
 *@brief:      mcu_i2c_start
 *@details:    发送start时序
 *@param[in]   void  
 *@param[out]  无
 *@retval:     static
 */
static void mcu_i2c_start(void) {
    mcu_i2c_sda_output();

    mcu_i2c_sda(1);
    mcu_i2c_scl(1);

    mcu_i2c_delay();
    mcu_i2c_sda(0);

    mcu_i2c_delay();
    mcu_i2c_scl(0);
}

/**
 *@brief:      mcu_i2c_stop
 *@details:    发送I2C STOP时序
 *@param[in]   void  
 *@param[out]  无
 *@retval:     static
 */
static void mcu_i2c_stop(void) {
    mcu_i2c_sda_output();

    mcu_i2c_scl(0);
    mcu_i2c_sda(0);
    mcu_i2c_delay();

    mcu_i2c_scl(1);
    mcu_i2c_delay();

    mcu_i2c_sda(1);
    mcu_i2c_delay();
}

/**
 *@brief:      mcu_i2c_wait_ack
 *@details:       等待ACK信号
 *@param[in]  void  
 *@param[out]  无
 *@retval:     static
 */
static s32 mcu_i2c_wait_ack(void) {
    u8 time_out = 0;

    //sda转输入
    mcu_i2c_sda_input();
    mcu_i2c_sda(1);
    mcu_i2c_delay();

    mcu_i2c_scl(1);
    mcu_i2c_delay();

    while (1) {
        time_out++;
        if (time_out > MCU_I2C_TIMEOUT) {
            mcu_i2c_stop();
            jprintf("i2c:wait ack time out!\r\n");
            return 1;
        }

        if (0 == mcu_i2c_readsda()) {
            break;
        }
    }

    mcu_i2c_scl(0);

    return 0;
}

/**
 *@brief:      mcu_i2c_ack
 *@details:       发送ACK信号
 *@param[in]  void  
 *@param[out]  无
 *@retval:     static
 */
static void mcu_i2c_ack(void) {
    mcu_i2c_scl(0);
    mcu_i2c_sda_output();

    mcu_i2c_sda(0);
    mcu_i2c_delay();

    mcu_i2c_scl(1);
    mcu_i2c_delay();

    mcu_i2c_scl(0);
}

/**
 *@brief:      mcu_i2c_writebyte
 *@details:       I2C总线写一个字节数据
 *@param[in]  u8 data  
 *@param[out]  无
 *@retval:     static
 */
static s32 mcu_i2c_writebyte(u8 data) {
    u8 i = 0;

    mcu_i2c_sda_output();

    mcu_i2c_scl(0);
    mcu_i2c_delay();
    while (i < 8) {

        if ((0x80 & data) == 0x80) {
            mcu_i2c_sda(1);
        } else {
            mcu_i2c_sda(0);
        }
        mcu_i2c_scl(1);
        mcu_i2c_delay();

        mcu_i2c_scl(0);
        mcu_i2c_delay();

        data = data << 1;
        i++;
    }
    return 0;
}

/**
 *@brief:      mcu_i2c_readbyte
 *@details:       I2C总线读一个字节数据
 *@param[in]  void  
 *@param[out]  无
 *@retval:     static
 */
static u8 mcu_i2c_readbyte(void) {
    u8 i = 0;
    u8 data = 0;

    mcu_i2c_sda_input();

    while (i < 8) {
        mcu_i2c_scl(0);
        mcu_i2c_delay();

        mcu_i2c_scl(1);

        data = (data << 1);

        if (1 == mcu_i2c_readsda()) {
            data = data | 0x01;
        } else {
            data = data & 0xfe;
        }

        mcu_i2c_delay();

        i++;
    }

    return data;
}

/**
 *@brief:      mcu_i2c_transfer
 *@details:    中间无重新开始位的传输流程
 *@param[in]   u8 addr   
               u8 rw    0 写，1 读    
               u8* data  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_i2c_transfer(u8 addr, u8 rw, u8 *data, s32 datalen) {
    s32 i;
    u8 ch;

#if 0//测试IO口是否连通
    while(1)
    {
        uart_printf("test \r\n");
        mcu_i2c_scl(1);
        mcu_i2c_sda(1); 
        Delay(5);
        mcu_i2c_scl(0);
        mcu_i2c_sda(0); 
        Delay(5);
    }
#endif

    //发送起始
    mcu_i2c_start();
    //发送地址+读写标志
    //处理ADDR
    if (rw == MCU_I2C_MODE_W) {
        addr = ((addr << 1) & 0xfe);
        //uart_printf("write\r\n");
    } else {
        addr = ((addr << 1) | 0x01);
        //uart_printf("read\r\n");
    }

    mcu_i2c_writebyte(addr);
    mcu_i2c_wait_ack();

    i = 0;
    while (i < datalen) {
        //数据传输
        if (rw == MCU_I2C_MODE_W)//写
        {
            ch = *(data + i);
            mcu_i2c_writebyte(ch);
            mcu_i2c_wait_ack();

        } else if (rw == MCU_I2C_MODE_R)//读
        {
            ch = mcu_i2c_readbyte();
            mcu_i2c_ack();
            *(data + i) = ch;
        }
        i++;
    }

    //发送结束
    mcu_i2c_stop();
    return 0;
}

#if 1

/**
 * I2C读取寄存器
 * @param addr 从机的设备地址，7位格式，不需要左移
 * @param reg_addr 要读取的寄存器地址
 * @param data 读出来的数据要放入的缓冲区
 * @param datalen 要读取的数据的长度
 * @return 0
 */
int32_t mcu_i2c_mem_read(u8 addr, uint8_t reg_addr, u8 *data, s32 datalen) {
    mcu_i2c_start();
    mcu_i2c_writebyte(a_w(addr)); //发送写寄存器的命令，用于写目标寄存器地址
    mcu_i2c_wait_ack();

    mcu_i2c_writebyte(reg_addr);  //写目标寄存器地址
    mcu_i2c_wait_ack();

    //根据数据手册，不停止，直接发送起始新的start
    mcu_i2c_start();

    mcu_i2c_writebyte(a_r(addr)); //发送写寄存器的命令，用于写目标寄存器地址
    mcu_i2c_wait_ack();
    for (int i = 0; i < datalen; ++i) {
        *data = mcu_i2c_readbyte();
        data++;
        if (i != datalen - 1)  //如果不是最后一个字节，就ack，如果是最后一个，不需要ack
            mcu_i2c_ack();
    }
    mcu_i2c_stop();
    return 0;
}


/**
 * I2C写入寄存器
 * @param addr 从机的设备地址，7位格式，不需要左移
 * @param reg_addr 要写入的寄存器地址
 * @param data 待写入缓冲区
 * @param datalen 数据的长度
 * @return 0
 */
int32_t mcu_i2c_mem_write(u8 addr, uint8_t reg_addr, u8 *data, s32 datalen) {
    mcu_i2c_start();
    mcu_i2c_writebyte(a_w(addr)); //发送写寄存器的命令，用于写目标寄存器地址
    mcu_i2c_wait_ack();

    mcu_i2c_writebyte(reg_addr);  //写目标寄存器地址
    mcu_i2c_wait_ack();

    for (int i = 0; i < datalen; ++i) {
        mcu_i2c_writebyte(*data);
        mcu_i2c_wait_ack();
        data++;
    }
    mcu_i2c_stop();
    return 0;
}


#endif

/**
 *@brief:      mcu_i2c_init
 *@details:    初始化I2C接口
 *@param[in]   void  
 *@param[out]  无
 *@retval:     
 */
s32 mcu_i2c_init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = MCU_I2C_SCL | MCU_I2C_SDA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式   
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(MCU_I2C_PORT, &GPIO_InitStructure);//初始化

    //初始化IO口状态
    mcu_i2c_scl(1);
    mcu_i2c_sda(1);

    return 0;
}


