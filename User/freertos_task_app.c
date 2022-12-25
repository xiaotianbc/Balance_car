//
// Created by xiaotian on 2022/12/24.
//

#include <stdio.h>
#include "main.h"
#include "mpu6050.h"
#include "lcd.h"
#include "GUI.h"
#include "src/core/lv_indev.h"
#include "examples/porting/lv_port_disp.h"
#include "demos/benchmark/lv_demo_benchmark.h"



QueueHandle_t evt;

uint32_t pub_SysTimerCount;
uint8_t bufs[128];

TaskHandle_t mpu6050_task;

MPU6050_t MPU6050;

void putchar_(char c) {
    SEGGER_RTT_PutChar(0, c);
}

#define MPU6050_ADDRESS_LEN  1         //MPU6050地址长度
#define MPU6050_ADDRESS     (0xD0>>1)  //MPU6050地址
#define MPU6050_WHO_AM_I     0x68U     //MPU6050 ID


#define MPU6050_GYRO_OUT        0x43
#define MPU6050_ACC_OUT         0x3B

#define ADDRESS_WHO_AM_I          (0x75U) // !< WHO_AM_I register identifies the device. Expected value is 0x68.
#define ADDRESS_SIGNAL_PATH_RESET (0x68U) // !<

//MPU6050寄存器
#define MPU_SELF_TESTX_REG        0x0D    //自检寄存器X
#define MPU_SELF_TESTY_REG        0x0E    //自检寄存器Y
#define MPU_SELF_TESTZ_REG        0x0F    //自检寄存器Z
#define MPU_SELF_TESTA_REG        0x10    //自检寄存器A
#define MPU_SAMPLE_RATE_REG        0x19    //采样频率分频器
#define MPU_CFG_REG                    0x1A    //配置寄存器
#define MPU_GYRO_CFG_REG          0x1B    //陀螺仪配置寄存器
#define MPU_ACCEL_CFG_REG          0x1C    //加速度计配置寄存器
#define MPU_MOTION_DET_REG        0x1F    //运动检测阀值设置寄存器
#define MPU_FIFO_EN_REG              0x23    //FIFO使能寄存器
#define MPU_I2CMST_CTRL_REG        0x24    //IIC主机控制寄存器
#define MPU_I2CSLV0_ADDR_REG    0x25    //IIC从机0器件地址寄存器
#define MPU_I2CSLV0_REG              0x26    //IIC从机0数据地址寄存器
#define MPU_I2CSLV0_CTRL_REG    0x27    //IIC从机0控制寄存器
#define MPU_I2CSLV1_ADDR_REG    0x28    //IIC从机1器件地址寄存器
#define MPU_I2CSLV1_REG              0x29    //IIC从机1数据地址寄存器
#define MPU_I2CSLV1_CTRL_REG    0x2A    //IIC从机1控制寄存器
#define MPU_I2CSLV2_ADDR_REG    0x2B    //IIC从机2器件地址寄存器
#define MPU_I2CSLV2_REG              0x2C    //IIC从机2数据地址寄存器
#define MPU_I2CSLV2_CTRL_REG    0x2D    //IIC从机2控制寄存器
#define MPU_I2CSLV3_ADDR_REG    0x2E    //IIC从机3器件地址寄存器
#define MPU_I2CSLV3_REG              0x2F    //IIC从机3数据地址寄存器
#define MPU_I2CSLV3_CTRL_REG    0x30    //IIC从机3控制寄存器
#define MPU_I2CSLV4_ADDR_REG    0x31    //IIC从机4器件地址寄存器
#define MPU_I2CSLV4_REG              0x32    //IIC从机4数据地址寄存器
#define MPU_I2CSLV4_DO_REG        0x33    //IIC从机4写数据寄存器
#define MPU_I2CSLV4_CTRL_REG    0x34    //IIC从机4控制寄存器
#define MPU_I2CSLV4_DI_REG        0x35    //IIC从机4读数据寄存器


#define MPU_PWR_MGMT1_REG          0x6B    //电源管理寄存器1
#define MPU_PWR_MGMT2_REG          0x6C    //电源管理寄存器2

#define MPU_I2CMST_STA_REG        0x36    //IIC主机状态寄存器
#define MPU_INTBP_CFG_REG          0x37    //中断/旁路设置寄存器
#define MPU_INT_EN_REG              0x38    //中断使能寄存器
#define MPU_INT_STA_REG              0x3A    //中断状态寄存器

#define MPU_I2CMST_DELAY_REG    0x67    //IIC主机延时管理寄存器
#define MPU_SIGPATH_RST_REG        0x68    //信号通道复位寄存器
#define MPU_MDETECT_CTRL_REG    0x69    //运动检测控制寄存器
#define MPU_USER_CTRL_REG          0x6A    //用户控制寄存器
#define MPU_PWR_MGMT1_REG          0x6B    //电源管理寄存器1
#define MPU_PWR_MGMT2_REG          0x6C    //电源管理寄存器2
#define MPU_FIFO_CNTH_REG          0x72    //FIFO计数寄存器高八位
#define MPU_FIFO_CNTL_REG          0x73    //FIFO计数寄存器低八位
#define MPU_FIFO_RW_REG              0x74    //FIFO读写寄存器
#define MPU_DEVICE_ID_REG          0x75    //器件ID寄存器

void mpu6050_write_reg(uint8_t reg_addr, uint8_t value) {
    mcu_i2c_mem_write(MPU6050_ADDRESS, reg_addr, &value, 1);
}

uint8_t mpu6050_read_reg(uint8_t reg_addr) {
    uint8_t val;
    mcu_i2c_mem_read(MPU6050_ADDRESS, reg_addr, &val, 1);
    return val;
}

void mpu6050_init(void) {
    mcu_i2c_init();
    mpu6050_write_reg(MPU_PWR_MGMT1_REG, 0x01);  //解除睡眠模式，选择时钟源为陀螺仪x轴时钟
    mpu6050_write_reg(MPU_PWR_MGMT2_REG, 0x00);  //
    mpu6050_write_reg(MPU_SAMPLE_RATE_REG, 0x09);//采样率 10分频
    mpu6050_write_reg(MPU_CFG_REG, 0x06);//最平滑的滤波
    mpu6050_write_reg(MPU_GYRO_CFG_REG, 0x18);//自测失能，量程选择最大，
    mpu6050_write_reg(MPU_ACCEL_CFG_REG, 0x18);//自测失能，量程选择最大，高通滤波器不使用
}

typedef struct {
    int16_t AccX;  //加速度
    int16_t AccY;
    int16_t AccZ;
    int16_t GyroX; //角速度
    int16_t GyroY;
    int16_t GyroZ;
    int16_t temp;
} mpu6050_data_t;

void mpu6050_get_data(mpu6050_data_t *r) {
    uint8_t DataRaw[14];
    mcu_i2c_mem_read(MPU6050_ADDRESS, MPU6050_ACC_OUT, DataRaw, 14);
    r->AccX = (DataRaw[0] << 8) | DataRaw[1];
    r->AccY = (DataRaw[2] << 8) | DataRaw[3];
    r->AccZ = (DataRaw[4] << 8) | DataRaw[5];
    r->temp = (DataRaw[6] << 8) | DataRaw[7];
    r->GyroX = (DataRaw[8] << 8) | DataRaw[9];
    r->GyroY = (DataRaw[10] << 8) | DataRaw[11];
    r->GyroZ = (DataRaw[12] << 8) | DataRaw[13];
}

void mpu6050_mainloop(void *arg) {
    // mpu6050_init();
    mcu_i2c_init();
    MPU6050_Init(NULL);

    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_SetBits(GPIOB, GPIO_Pin_8);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//普通输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//上拉
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化


    while (1) {
        GPIO_ResetBits(GPIOB, GPIO_Pin_8);
        //   MPU6050_Read_Accel(0,&MPU6050);
        //   MPU6050_Read_Gyro(0,&MPU6050);
        //   MPU6050_Read_Temp(0,&MPU6050);
        MPU6050_Read_All(0, &MPU6050);
        GPIO_SetBits(GPIOB, GPIO_Pin_8);
        printf("Acc  XYZ: %+d   %+d   %+d\n", MPU6050.Accel_X_RAW, MPU6050.Accel_Y_RAW, MPU6050.Accel_Z_RAW);
        printf("Gyro XYZ: %+d   %+d   %+d\n", MPU6050.Gyro_X_RAW, MPU6050.Gyro_Y_RAW, MPU6050.Gyro_Z_RAW);
        printf("Temp:  %.2f\n\n", MPU6050.Temperature);
        vTaskDelay(100);
    }


    vTaskDelete(NULL);

    uint8_t i = 0;
    while (MPU6050_Init(NULL) == 1);

    while (1) {
        pub_SysTimerCount = SysTick->VAL;
        MPU6050_Read_All(NULL, &MPU6050);
        pub_SysTimerCount = SysTick->VAL > pub_SysTimerCount ? (SysTick->VAL - pub_SysTimerCount) : (pub_SysTimerCount -
                                                                                                     (SysTick->VAL));
        xQueueSend(evt, &i, portMAX_DELAY);
        vTaskDelay(100);
    }
}


void lcd_disp_bmp16(uint16_t xpos, uint16_t ypos, uint8_t *pbmp) {
    //获取bmp文件里，真实像素地址的位移，一般是在文件头起始偏移0x0A个字节的长度
    uint32_t buffer_offset = *(uint32_t *) (pbmp + 0x0A);
    uint32_t pic_width = *(uint32_t *) (pbmp + 0x12);
    uint32_t pic_height = *(uint32_t *) (pbmp + 0x16);

    printf("buffer offset:0x%lX\n", buffer_offset);
    printf("pic_width:%lu\n", pic_width);
    printf("pic_height:%lu\n", pic_height);

    pbmp = pbmp + buffer_offset;  //移动到真实数据地址

    uint16_t pic_x_size = pic_width;
    uint16_t pic_y_size = pic_height;
    uint16_t lcd_w = 240;
    uint16_t lcd_h = 240;
    //LCD_WriteReg(0x36, (1 << 6) | (1 << 7));//BGR==1,MY==0,MX==0,MV==0
    LCD_WriteReg(0x36, (1 << 6) | (1 << 5));//BGR==1,MY==1,MX==0,MV==1
    /**
     * Magic number? 暂时不知道为啥是这样值，反正是硬调出来的。。。
     */
    xpos = lcd_w - 80 - 48 - xpos;
    ypos = ypos + 80;

    LCD_SetWindows(xpos, ypos, xpos + pic_x_size - 1, ypos + pic_y_size - 1);//窗口设置
    int i;
    for (i = 0; i < pic_x_size * pic_y_size; i++) {
        Lcd_WriteData_16Bit(*(uint16_t *) (pbmp + i * 2));
    }
    LCD_SetWindows(0, 0, lcddev.width - 1, lcddev.height - 1);//恢复显示窗口为全屏
    LCD_WriteReg(0x36, 0);//BGR==1,MY==0,MX==0,MV==0
}


//定时器回调函数
void lv_tick_inc_timer(TimerHandle_t xTimer) {
    lv_tick_inc(10);
}

LV_IMG_DECLARE(lena_hires);
LV_IMG_DECLARE(lenap);
lv_obj_t *img1;
lv_obj_t *img2;


//定时器回调函数
void lv_switch_page_timer(TimerHandle_t xTimer) {
    static uint8_t flag = 0;
    if (flag) {
        lv_obj_add_flag(img1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(img2, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(img2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(img1, LV_OBJ_FLAG_HIDDEN);
    }
    flag = !flag;
}

void lvgl_user_app(void) {
    img1 = lv_img_create(lv_scr_act());
    lv_img_set_src(img1, &lenap);
    lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(img1, 240, 240);

    img2 = lv_img_create(lv_scr_act());
    lv_img_set_src(img2, &lena_hires);
    lv_obj_align(img2, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(img2, 240, 240);

    TimerHandle_t timer_handle2 = xTimerCreate("lv_switch_page_timer",//定时器名称
                                               1000,//定时器计时周期，计时结束后触发回调函数
                                               pdTRUE,//是否自动重装载定时器
                                               NULL, //定时器ID
                                               lv_switch_page_timer //定时器回调函数
    );
    xTimerStart(timer_handle2, portMAX_DELAY);


}


void lcd_disp(void *a) {
    uint32_t flush_cnt = 0;
    size_t freeHeap;
    UBaseType_t waterMark;

    LCD_Init();
    lv_init();
    lv_port_disp_init();

    lvgl_user_app();
    //lv_demo_benchmark();

    TimerHandle_t timer_handle; //定义一个定时器句柄
    timer_handle = xTimerCreate("print_timer",//定时器名称
                                10,//定时器计时周期，计时结束后触发回调函数
                                pdTRUE,//是否自动重装载定时器
                                NULL, //定时器ID
                                lv_tick_inc_timer //定时器回调函数
    );
    xTimerStart(timer_handle, portMAX_DELAY);


    waterMark = uxTaskGetStackHighWaterMark(NULL);
    sprintf((char *) bufs, "LCD-freeMem: %lu byte\n", waterMark * 4);//栈深度*4=字节数
    SEGGER_RTT_printf(0, "%s", bufs);

    while (1) {
        lv_timer_handler();
        vTaskDelay(10);
    }

    uint8_t dum = 0;
    while (1) {
        //等待获取新的消息，获取到之后，对应的标志位会自动被清除，不需要手动清除
        xQueueReceive(evt, &dum, portMAX_DELAY);
        //     sprintf(bufs, "Axyz: %+04.2lf %+04.2lf %+04.2lf \n", MPU6050.Ax, MPU6050.Ay,
        //            MPU6050.Az);
        //     SEGGER_RTT_printf(0, "%s", bufs);
        //   sprintf(bufs, "Gxyz: %+04.2lf %+04.2lf %+04.2lf\n", MPU6050.Gx, MPU6050.Gy,
        //            MPU6050.Gz);
        sprintf((char *) bufs, "GRAW: %d %d %d\n", MPU6050.Gyro_X_RAW, MPU6050.Gyro_Y_RAW,
                MPU6050.Gyro_Z_RAW);
        SEGGER_RTT_printf(0, "%s\n", bufs);

        freeHeap = xPortGetFreeHeapSize();

        sprintf((char *) bufs, "freeHeap: %zu byte\n", freeHeap);
        SEGGER_RTT_printf(0, "%s", bufs);


        waterMark = uxTaskGetStackHighWaterMark(NULL);
        sprintf((char *) bufs, "LCD-freeMem: %lu byte\n", waterMark * 4);//栈深度*4=字节数
        SEGGER_RTT_printf(0, "%s", bufs);

        waterMark = uxTaskGetStackHighWaterMark(mpu6050_task);
        sprintf((char *) bufs, "MPU-freeMem: %lu byte\n", waterMark * 4);//栈深度*4=字节数
        SEGGER_RTT_printf(0, "%s", bufs);
        board_led_toggle();
        // jprintf("uhADCxConvertedValue: %u \n", uhADCxConvertedValue);
        //分压系数6.22， VddMCU=3.4  6.22*3.4=21.15
        sprintf((char *) bufs, "Vbat: %.2f V\n", get_vbat_value());
        SEGGER_RTT_printf(0, "%s", bufs);

        extern uint8_t motorl_speed;
        sprintf((char *) bufs, "Lspeed: %d %%\n", motorl_speed);
        SEGGER_RTT_printf(0, "%s", bufs);
        SEGGER_RTT_printf(0, "******over*******\n\n");
    }
}
