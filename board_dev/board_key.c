#include "board_key.h"
#include "stm32f4xx.h"

/**
 * 板上按键初始化
 * 我设计的这块板上只有1个按键：是PA8
 * 这里再假设PA11, PA12是按键，这样总共就有3个按键
 * 按键不按的时候，悬空，按下后接地
 */


/**
 * @brief 初始化板载的按钮
 *
 */
void board_key_init(void) {
    /*开启GPIOA 外设时钟*/
    RCC->AHB1ENR |= (1 << 0);

    /*把PA8 MODERy[1:0]配置为输入模式：00即可，其实复位后默认就是输入模式 */
    GPIOA->MODER &= ~(3 << (8 * 2)); //清空
    /*把PA11 MODERy[1:0]配置为输入模式：00即可，其实复位后默认就是输入模式 */
    GPIOA->MODER &= ~(3 << (11 * 2)); //清空
    /*把PA12 MODERy[1:0]配置为输入模式：00即可，其实复位后默认就是输入模式 */
    GPIOA->MODER &= ~(3 << (12 * 2)); //清空


    //GPIOx_OTYPER 端口输出类型寄存器，按键输入模式不用管

    //GPIOx_OSPEEDR 端口输出速度寄存器，按键输入模式不用管


    /*PUPDR上下拉寄存器，按下后是接地，所以配置为上拉输入：01<<0*/
    GPIOA->PUPDR |= (1 << (8 * 2));
    GPIOA->PUPDR |= (1 << (11 * 2));
    GPIOA->PUPDR |= (1 << (12 * 2));

}

/**
 * @brief 读取板载按钮是否被按下
 *
 * @return uint8_t 按下为1，没有按下为0
 */
uint8_t board_key_pressed(KEY_enum key) {
    uint8_t ret = 0;
    /*因为按下时是低电平，所以这里对读取的IDR做一个取反操作*/
    switch (key) {
        case KEY1:
            ret = !((GPIOA->IDR) & (1 << 8));
            break;
        case KEY2:
            ret = !((GPIOA->IDR) & (1 << 11));
            break;
        case KEY3:
            ret = !((GPIOA->IDR) & (1 << 12));
            break;
        default:
            break;
    }
    return ret;
}

/**
 * 假的延时函数，大约效果是延时20ms左右
 */
void delay_20ms(void) {
    uint32_t cnt = 20 * 12000;
    while (cnt--);
}

/**
 * 按键扫描函数，轮询扫描开发板上的哪个按键被按下了，松手时再返回
 * @return 类型为KEY_enum，如果没有按键按下就是NOKEY，否则是KEY1-4
 */
KEY_enum board_key_scan(void) {
    KEY_enum key_num = NOKEY; //用于返回被按下的按键是哪个按键
    //依次轮询
    for (KEY_enum k = KEY1; k <= KEY3; k++) {
        //判断按键当前是否被按下
        if (board_key_pressed(k)) {
            //如果被按下，再等20毫秒
            delay_20ms();
            //此时如果没有松手，就在这里等着，直到松手为止
            while (board_key_pressed(k));
            //松手之后，防止松手抖动，再等20ms
            delay_20ms();
            key_num = k;
            return key_num;
        }
    }
    return key_num;
}