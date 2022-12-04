/**
串口作业
本程序实现的功能：电脑发送一段字符串给串口，串口再回复电脑相同的字符串
*/

#include <stdio.h>
#include "main.h"
#include "cmsis_os2.h"
#include "mcu_i2c_soft.h"
#include "lcd.h"
#include "GUI.h"

#define FLAGS_MSK1_GOT_NEW_DATA 0x00000001U
osEventFlagsId_t evt_id;                        // event flasg id

uint32_t pub_SysTimerCount;
uint8_t bufs[128];

void mpu6050_mainloop(void *arg) {
    uint8_t addr[20] = {0};
    mcu_i2c_init();
    uint8_t i = 0;
    mcu_i2c_mem_write(0x68, 0x6B, &i, 1); //上电后首先解除SLEEP模式

    while (1) {
        pub_SysTimerCount = osKernelGetSysTimerCount();
        mcu_i2c_mem_read(0x68, 0x3b, addr, 5);
        pub_SysTimerCount = osKernelGetSysTimerCount() - pub_SysTimerCount;
        sprintf(bufs, "I2C: %02X %02X %02X %02X %02X, %d us", addr[0], addr[1],
                addr[2], addr[3], addr[4],
                pub_SysTimerCount / 120);
        osEventFlagsSet(evt_id, FLAGS_MSK1_GOT_NEW_DATA);  //给时间的Mask置位
        // osThreadYield();                            // suspend thread

        osDelay(100);
    }
}

void lcd_disp(void *a) {
    LCD_Init();
    //绘制固定栏up
    LCD_Clear(GREEN);
    LCD_Fill(0, 0, lcddev.width, 20, WHITE);
    POINT_COLOR = WHITE;
    Gui_StrCenter(0, 2, BLACK, BLUE, "MPU6050-TEST", 16, 1);//居中显示

    uint32_t flush_cnt = 0;
    while (1) {
        //等待获取新的消息，获取到之后，对应的标志位会自动被清除，不需要手动清除
        osEventFlagsWait(evt_id, FLAGS_MSK1_GOT_NEW_DATA, osFlagsWaitAny, osWaitForever);
        // osEventFlagsClear (evt_id, FLAGS_MSK1_GOT_NEW_DATA);

        pub_SysTimerCount = osKernelGetSysTimerCount();
        LCD_Fill(0, 20, lcddev.width, 36, GREEN);
        Gui_StrCenter(0, 22, BLACK, BLUE, bufs, 16, 1);//居中显示
        pub_SysTimerCount = osKernelGetSysTimerCount() - pub_SysTimerCount;

        sprintf(bufs, "%lu ' /%d us", flush_cnt++, pub_SysTimerCount / 120);
        LCD_Fill(0, 37, lcddev.width, 37 + 16, GREEN);
        Gui_StrCenter(0, 37, BLACK, BLUE, bufs, 16, 1);//居中显示


        uint32_t cStackSize = osThreadGetCount();
        sprintf(bufs, "StackCount: %d", cStackSize);
        LCD_Fill(0, 54, lcddev.width, 54 + 16, GREEN);
        Gui_StrCenter(0, 54, BLACK, BLUE, bufs, 16, 1);//居中显示

        board_led_toggle();
    }
}


int main(void) {
    NVIC_SetPriorityGrouping(4);  //配置使用4bits 作为NVIC中断优先级分组，此函数程序只执行一次

    osKernelInitialize();                 // Initialize CMSIS-RTOS
    //初始化segger rtt,使用NO_BLOCK_SKIP模式，当没有读取时，把数据丢弃，程序不会被卡死
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);

    board_key_init();  //初始化按键
    board_led_init();   //初始化LED
    board_uart1_init(115200);  //初始化串口1，开启接收中断加入环形缓冲区
    mcu_tim11_init();

    evt_id = osEventFlagsNew(NULL);
    osThreadNew(mpu6050_mainloop, NULL, NULL);    // Create application main thread
    osThreadNew(lcd_disp, NULL, NULL);    // Create application main thread

    // osThreadNew(app_main, NULL, NULL);    // Create application main thread
    osKernelStart();                      // Start thread execution
    for (;;) {
        osDelay(osWaitForever);
    }
}


