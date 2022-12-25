#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "core_cm4.h"
#include "board_uart1.h"
#include "board_led.h"
#include "rb.h"
#include "mcu_tim11.h"
#include "board_key.h"
#include "string.h"
#include "printf.h"

#include "mcu_i2c_soft.h"
#include "board_bat_adc.h"
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/


#ifndef delay_ms
#define delay_ms Delay
#endif

#define  SETBIT(REG, BIT)     ((REG) |= (BIT))
#define  CLRBIT(REG, BIT)   ((REG) &= ~(BIT))

#include "SEGGER_RTT.h"
#define jprintf(args...)   SEGGER_RTT_printf(0, ##args)

/*关于时间：主频120MHz的情况下，1秒钟有120*1000*1000个Tick*/

/* 开关全局中断的宏 */
#define ENABLE_INT()	__set_PRIMASK(0)	/* 使能全局中断 */
#define DISABLE_INT()	__set_PRIMASK(1)	/* 禁止全局中断 */
///* 开关全局中断的宏 */
//#define ENABLE_INT()		/* 使能全局中断 */
//#define DISABLE_INT()		/* 禁止全局中断 */


#define LCD_DC_Pin GPIO_Pin_14
#define LCD_DC_GPIO_Port GPIOC
#define LCD_RESET_Pin GPIO_Pin_15
#define LCD_RESET_GPIO_Port GPIOC
#define USER_LED_Pin GPIO_Pin_2
#define USER_LED_GPIO_Port GPIOB
#define MPU_INT_Pin GPIO_Pin_4
#define MPU_INT_GPIO_Port GPIOB
#define LCD_BLK_Pin GPIO_Pin_8
#define LCD_BLK_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_Pin_9
#define LCD_CS_GPIO_Port GPIOB


/* Exported functions ------------------------------------------------------- */

#endif /* __MAIN_H */

