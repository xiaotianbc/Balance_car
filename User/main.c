/**
串口作业
本程序实现的功能：电脑发送一段字符串给串口，串口再回复电脑相同的字符串
*/

#include <stdio.h>
#include "main.h"
#include "mpu6050.h"
#include "lcd.h"
#include "GUI.h"
#include "src/core/lv_indev.h"
#include "examples/porting/lv_port_disp.h"
#include "demos/benchmark/lv_demo_benchmark.h"

extern QueueHandle_t evt;

void lcd_disp(void *a);


#include <sfud.h>


#define SFUD_DEMO_TEST_BUFFER_SIZE                     1024

static void sfud_demo(uint32_t addr, size_t size, uint8_t *data);

static uint8_t sfud_demo_test_buf[SFUD_DEMO_TEST_BUFFER_SIZE];


/**
 * SFUD demo for the first flash device test.
 *
 * @param addr flash start address
 * @param size test flash size
 * @param size test flash data buffer
 */
static void sfud_demo(uint32_t addr, size_t size, uint8_t *data) {
    sfud_err result = SFUD_SUCCESS;
    const sfud_flash *flash = sfud_get_device_table() + 0;
    size_t i;
    /* prepare write data */
    for (i = 0; i < size; i++) {
        data[i] = i;
    }
    /* erase test */
    result = sfud_erase(flash, addr, size);
    if (result == SFUD_SUCCESS) {
        printf("Erase the %s flash data finish. Start from 0x%08X, size is %ld.\r\n", flash->name, addr,
               size);
    } else {
        printf("Erase the %s flash data failed.\r\n", flash->name);
        return;
    }
    /* write test */
    result = sfud_write(flash, addr, size, data);
    if (result == SFUD_SUCCESS) {
        printf("Write the %s flash data finish. Start from 0x%08X, size is %ld.\r\n", flash->name, addr,
               size);
    } else {
        printf("Write the %s flash data failed.\r\n", flash->name);
        return;
    }
    /* read test */
    result = sfud_read(flash, addr, size, data);
    if (result == SFUD_SUCCESS) {
        printf("Read the %s flash data success. Start from 0x%08X, size is %ld. The data is:\r\n", flash->name, addr,
               size);
        printf("Offset (h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\r\n");
        for (i = 0; i < size; i++) {
            if (i % 16 == 0) {
                printf("[%08X] ", addr + i);
            }
            printf("%02X ", data[i]);
            if (((i + 1) % 16 == 0) || i == size - 1) {
                printf("\r\n");
            }
        }
        printf("\r\n");
    } else {
        printf("Read the %s flash data failed.\r\n", flash->name);
    }
    /* data check */
    for (i = 0; i < size; i++) {
        if (data[i] != i % 256) {
            printf("Read and check write data has an error. Write the %s flash data failed.\r\n", flash->name);
            break;
        }
    }
    if (i == size) {
        printf("The %s flash test is success.\r\n", flash->name);
    }
}

static uint8_t bufs[128];

void print_mem(const uint8_t *mem_addr, uint32_t len) {
    for (uint32_t i = 0; i < len; ++i) {
        if (i % 16 == 0)
            printf("\n");
        printf("%02X ", *(mem_addr + i));
    }
}

#include "lfs.h"
#include "lfs_port.h"

// variables used by the filesystem
lfs_t lfs;
lfs_file_t file;

// configuration of the filesystem is provided by this struct
const struct lfs_config cfg = {
        // block device operations
        .read  = W25Qxx_readLittlefs,
        .prog  = W25Qxx_writeLittlefs,
        .erase = W25Qxx_eraseLittlefs,
        .sync  = W25Qxx_syncLittlefs,

        // block device configuration
        .read_size = 256,
        .prog_size = 256,
        .block_size = W25Q128_ERASE_GRAN,
        .block_count = W25Q128_NUM_GRAN,
        .cache_size = 512,
        .lookahead_size = 512,
        .block_cycles = 500,
};

void spisd_task(void *a) {
    lfs_port_init();

    int err = lfs_mount(&lfs, &cfg);
    printf("lfs_mount=%d\n", err);

    if (err) {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
        printf("lfs_mount in err=%d\n", err);
    }

    while (1) {
        uint32_t boot_count = 0;
        lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
        lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

        // update boot count
        boot_count += 1;
        lfs_file_rewind(&lfs, &file);  // seek the file to begin
        lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

        // remember the storage is not updated until the file is closed successfully
        lfs_file_close(&lfs, &file);

//		// release any resources we were using
        //	lfs_unmount(&lfs);

        // print the boot count
        printf("boot_count: %d\n", boot_count);

        vTaskDelay(20000);
    }

    vTaskDelete(NULL);
}


int main(void) {
    NVIC_SetPriorityGrouping(4);  //配置使用4bits 作为NVIC中断优先级分组，此函数程序只执行一次

    //初始化segger rtt,SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL，防止数据丢失
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);

    board_key_init();  //初始化按键
    board_led_init();   //初始化LED
    board_uart1_init(115200);  //初始化串口1，开启接收中断加入环形缓冲区
    mcu_tim11_init();
    ADC_open();
    evt = xQueueCreate(1, 1);
    extern void motor_run(void *);
    //  xTaskCreate(mpu6050_mainloop, "MPU6050", 4096 / 4, NULL, 1, &mpu6050_task);
//    xTaskCreate(lcd_disp, "MPU6050", 8192 / 4, NULL, 1, NULL);
    xTaskCreate(spisd_task, "spisd_task", 8192 / 4, NULL, 1, NULL);
    //  xTaskCreate(motor_run, "MPU6050", 1024 / 4, NULL, 4, NULL);
    vTaskStartScheduler();                      // Start thread execution
    for (;;) {
        // osDelay(osWaitForever);
    }
}


