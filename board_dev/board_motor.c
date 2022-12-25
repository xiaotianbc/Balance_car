//
// Created by xiaotian on 2022/12/21.
//

#include "board_motor.h"

//MS 123 全部接地=1细分

/**
 * 根据A4988的数据手册，每次每次STEP脉冲要在DIR换向前后的200ns以上
 * STEP 高低电平大于1us
 */

//PA1 -> TIM5-CH2
#define  MOTORL_STEP_GPIOx GPIOA
#define  MOTORL_STEP_Pin GPIO_Pin_1
#define  MOTORL_STEP_PinSourceX GPIO_PinSource1

//DIR: 低电平正转，高电平反转
#define  MOTORL_DIR_GPIOx GPIOB
#define  MOTORL_DIR_Pin GPIO_Pin_1

//enable pb0
#define  MOTOR_ENABLE_GPIOx GPIOB
#define  MOTORL_ENABLE_Pin GPIO_Pin_0


/**
 * 配置STEP pin 作为TIM5
 */
static void TIM_GPIO_Config(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    /* TIM5 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

    /* GPIOC clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = MOTORL_STEP_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(MOTORL_STEP_GPIOx, &GPIO_InitStructure);
    GPIO_PinAFConfig(MOTORL_STEP_GPIOx, MOTORL_STEP_PinSourceX, GPIO_AF_TIM5);
}


/* Private variables ---------------------------------------------------------*/
static TIM_TimeBaseInitTypeDef MOTORL_TIM_TimeBaseStructure;
static TIM_OCInitTypeDef TIM_OCInitStructure;
static uint16_t PrescalerValue = 0;

void tim_motor_run(void) {
    motor_gpio_init();

    //低电平电机工作，高电平不工作
    GPIO_ResetBits(MOTOR_ENABLE_GPIOx, MOTORL_ENABLE_Pin);
    //低电平正转，高电平反转
    GPIO_ResetBits(MOTORL_DIR_GPIOx, MOTORL_DIR_Pin);


    /* TIM Configuration */
    TIM_GPIO_Config();

#define MOTORL_STEP_TIMx TIM5

    /* -----------------------------------------------------------------------
      TIM3 Configuration: generate 4 PWM signals with 4 different duty cycles.

      In this example TIM3 input clock (TIM3CLK) is set to 2 * APB1 clock (PCLK1),
      since APB1 prescaler is different from 1.
        TIM3CLK = 2 * PCLK1
        PCLK1 = HCLK / 2
        => TIM3CLK = HCLK = SystemCoreClock

      To get TIM3 counter clock at 21 MHz, the prescaler is computed as follows:
         Prescaler = (TIM3CLK / TIM3 counter clock) - 1
         Prescaler = ((SystemCoreClock /2) /21 MHz) - 1

      To get TIM3 output clock at 30 KHz, the period (ARR)) is computed as follows:
         ARR = (TIM3 counter clock / TIM3 output clock) - 1
             = 665

      TIM3 Channel1 duty cycle = (TIM3_CCR1/ TIM3_ARR)* 100 = 50%
      TIM3 Channel2 duty cycle = (TIM3_CCR2/ TIM3_ARR)* 100 = 37.5%
      TIM3 Channel3 duty cycle = (TIM3_CCR3/ TIM3_ARR)* 100 = 25%
      TIM3 Channel4 duty cycle = (TIM3_CCR4/ TIM3_ARR)* 100 = 12.5%
    ----------------------------------------------------------------------- */

    /* Compute the prescaler value */
    //设置TIM5 counter clock 为 1MHz
    PrescalerValue = (uint16_t) ((SystemCoreClock) / 1000000) - 1;

    /* Time base configuration */
    //要设置的输出频率为2000Hz , Period=TIM5 counter clock/500Hz -1 = 1999
    MOTORL_TIM_TimeBaseStructure.TIM_Period = 299;
    MOTORL_TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
    MOTORL_TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    MOTORL_TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(MOTORL_STEP_TIMx, &MOTORL_TIM_TimeBaseStructure);


    /**
     * PWM模式1下，TIMx_CNT<TIMx_CCRn时，输出有效电平
                         TIMx_CNT>TIMx_CCRn时，输出无效电平
        PWM模式2下，TIMx_CNT<TIMx_CCRn时，输出无效电平
                         TIMx_CNT>TIMx_CCRn时，输出有效电平

有效电平由TIM_OCInitStructure.TIM_OCPolarity决定
可以是TIM_OCPolarity_High（高电平）或者TIM_OCPolarity_Low（低电平）
     */

    /* PWM1 Mode configuration: Channel2 */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 149;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;

    TIM_OC2Init(MOTORL_STEP_TIMx, &TIM_OCInitStructure);

    TIM_OC2PreloadConfig(MOTORL_STEP_TIMx, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(MOTORL_STEP_TIMx, ENABLE);

    /* TIM3 enable counter */
    TIM_Cmd(MOTORL_STEP_TIMx, ENABLE);
}

/**
 * 设置左边电机的速度
 * @param p 速度百分比，1-100
 */
void motorl_set_speed(uint8_t p) {
    //最少要10%
    if (p < 10)
        p = 10;
    uint32_t speed = 29900 / p;
    MOTORL_TIM_TimeBaseStructure.TIM_Period = speed;
    TIM_TimeBaseInit(MOTORL_STEP_TIMx, &MOTORL_TIM_TimeBaseStructure);
    TIM_SetCompare2(MOTORL_STEP_TIMx, speed / 2 - 1);
    TIM_SetCounter(MOTORL_STEP_TIMx, 0);
}

void motorl_start(){
    TIM_Cmd(MOTORL_STEP_TIMx, ENABLE);
}

void motorl_stop(){
    TIM_Cmd(MOTORL_STEP_TIMx, DISABLE);
}


void motor_gpio_init(void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    /* GPIOG Peripheral clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    //低电平电机工作，高电平不工作
    GPIO_SetBits(MOTOR_ENABLE_GPIOx, MOTORL_ENABLE_Pin);
    /* L STEP */
    GPIO_InitStructure.GPIO_Pin = MOTORL_STEP_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    //  GPIO_Init(MOTORL_STEP_GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = MOTORL_DIR_Pin;
    GPIO_Init(MOTORL_DIR_GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = MOTORL_ENABLE_Pin;
    GPIO_Init(MOTOR_ENABLE_GPIOx, &GPIO_InitStructure);
}

uint8_t motorl_speed = 100;

void motor_run(void) {

    tim_motor_run();
    uint8_t dec_flag = 1;
    while (1){
        motorl_start();
        vTaskDelay(20);
        motorl_stop();vTaskDelay(20);
    }

    while (1) {
        motorl_set_speed(motorl_speed);
        vTaskDelay(500);
        if (dec_flag) {
            motorl_speed -= 5;
        } else {
            motorl_speed += 5;
        }

        if (motorl_speed < 40) {
            vTaskDelay(2000);
            dec_flag = 0;
        }
        if (motorl_speed > 100) {
            vTaskDelay(2000);
            dec_flag = 1;
        }

    }

    vTaskDelete(NULL);

    motor_gpio_init();
    //低电平电机工作，高电平不工作
    GPIO_ResetBits(MOTOR_ENABLE_GPIOx, MOTORL_ENABLE_Pin);

    //低电平正转，高电平反转
    GPIO_ResetBits(MOTORL_DIR_GPIOx, MOTORL_DIR_Pin);

    while (1) {
        GPIO_ToggleBits(MOTORL_STEP_GPIOx, MOTORL_STEP_Pin);
        vTaskDelay(1);
    }
}