//
// Created by xiaotian on 2022/11/2.
//
#include "stm32f4xx.h"
#include "board_led.h"
#include "main.h"

/**
 * 初始化主板上的PB2
 */
void board_led_init(void) {
    //LED PB2 低电平点亮

    //使能GPIOB时钟
    RCC->AHB1ENR |= (1 << 1);

    /*LED灯，低电平点亮，初始化时先将其修改为高电平，熄灭状态*/
    /*在配置为输出模式之前修改输出电平，可以避免上电一瞬间LED灯点亮后再熄灭*/
    GPIOB->ODR |= (1 << 1);

    /*把PG0,PG1,PG2,PG3 MODERy[1:0]配置为01b，为通用输出模式*/
    GPIOB->MODER &= ~(3 << (2 * 2)); //先清空
    GPIOB->MODER |= (1 << (2 * 2));  //再写入

    /*GPIOx_OTYPER复位后默认状态即为推挽输出，不需要修改*/

    /*修改OSPEEDR为最高速度*/
    GPIOB->OSPEEDR |= (3 << (2 * 2));

    /*PUPDR上下拉寄存器，复位后默认为不上下拉，不需要修改*/

    board_led_off();
}

/**
 * 关灯
 */
void board_led_off(void) {
    GPIOB->ODR |= (1 << 2);
}

/**
 * 开灯
 */
void board_led_on(void) {
    GPIOB->ODR &= ~(1 << 2);
}

void board_led_toggle(void) {
    GPIOB->ODR^=1<<2;
}