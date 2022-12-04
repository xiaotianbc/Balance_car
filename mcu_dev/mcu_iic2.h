//
// Created by xiaotian on 2022/12/1.
//

#ifndef F401_BALANCE_CAR_BOOTLOADER_MCU_IIC2_H
#define F401_BALANCE_CAR_BOOTLOADER_MCU_IIC2_H

#if 0

#include "main.h"

/*I2C 和DMA相关配置*/

/** @addtogroup STM324xG_EVAL_LOW_LEVEL_I2C_EE
  * @{
  */
/**
  * @brief  I2C EEPROM Interface pins
  */
#define sEE_I2C                          I2C2
#define sEE_I2C_CLK                      RCC_APB1Periph_I2C2
#define sEE_I2C_SCL_PIN                  GPIO_Pin_10                  /* PB.10 */
#define sEE_I2C_SCL_GPIO_PORT            GPIOB                       /* GPIOB */
#define sEE_I2C_SCL_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define sEE_I2C_SCL_SOURCE               GPIO_PinSource10
#define sEE_I2C_SCL_AF                   GPIO_AF_I2C2
#define sEE_I2C_SDA_PIN                  GPIO_Pin_3                 /* PB.03 */
#define sEE_I2C_SDA_GPIO_PORT            GPIOB                       /* GPIOB */
#define sEE_I2C_SDA_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define sEE_I2C_SDA_SOURCE               GPIO_PinSource3
#define sEE_I2C_SDA_AF                   GPIO_AF_I2C2
//#define sEE_M24C64_32
#define sEE_M24C08
//I2C2_RX Stream 2, I2C2_TX Stream 7, DR_Address  0x40005810

#define sEE_I2C_DMA                      DMA1
#define sEE_I2C_DMA_CHANNEL              DMA_Channel_7
#define sEE_I2C_DMA_STREAM_TX            DMA1_Stream7
#define sEE_I2C_DMA_STREAM_RX            DMA1_Stream2
#define sEE_I2C_DMA_CLK                  RCC_AHB1Periph_DMA1
#define sEE_I2C_DR_Address               (&(I2C2->DR))
#define sEE_USE_DMA

#define sEE_I2C_DMA_TX_IRQn              DMA1_Stream7_IRQn
#define sEE_I2C_DMA_RX_IRQn              DMA1_Stream2_IRQn
#define sEE_I2C_DMA_TX_IRQHandler        DMA1_Stream7_IRQHandler
#define sEE_I2C_DMA_RX_IRQHandler        DMA1_Stream2_IRQHandler
#define sEE_I2C_DMA_PREPRIO              0
#define sEE_I2C_DMA_SUBPRIO              0

#define sEE_TX_DMA_FLAG_FEIF             DMA_FLAG_FEIF7
#define sEE_TX_DMA_FLAG_DMEIF            DMA_FLAG_DMEIF7
#define sEE_TX_DMA_FLAG_TEIF             DMA_FLAG_TEIF7
#define sEE_TX_DMA_FLAG_HTIF             DMA_FLAG_HTIF7
#define sEE_TX_DMA_FLAG_TCIF             DMA_FLAG_TCIF7
#define sEE_RX_DMA_FLAG_FEIF             DMA_FLAG_FEIF2
#define sEE_RX_DMA_FLAG_DMEIF            DMA_FLAG_DMEIF2
#define sEE_RX_DMA_FLAG_TEIF             DMA_FLAG_TEIF2
#define sEE_RX_DMA_FLAG_HTIF             DMA_FLAG_HTIF2
#define sEE_RX_DMA_FLAG_TCIF             DMA_FLAG_TCIF2

#define sEE_DIRECTION_TX                 0
#define sEE_DIRECTION_RX                 1

/* Time constant for the delay caclulation allowing to have a millisecond
   incrementing counter. This value should be equal to (System Clock / 1000).
   ie. if system clock = 168MHz then sEE_TIME_CONST should be 168. */
#define sEE_TIME_CONST                   168
/**
  * @}
  */
/**
  * @}
  */
void sEE_LowLevel_DeInit(void);
void sEE_LowLevel_Init(void);
void sEE_LowLevel_DMAConfig(uint32_t pBuffer, uint32_t BufferSize, uint32_t Direction);


/* Uncomment the following line to use the default sEE_TIMEOUT_UserCallback()
   function implemented in stm32_evel_i2c_ee.c file.
   sEE_TIMEOUT_UserCallback() function is called whenever a timeout condition
   occure during communication (waiting on an event that doesn't occur, bus
   errors, busy devices ...). */
/* #define USE_DEFAULT_TIMEOUT_CALLBACK */

#if !defined (sEE_M24C08) && !defined (sEE_M24C64_32)
/* Use the defines below the choose the EEPROM type */
/* #define sEE_M24C08*/  /* Support the device: M24C08. */
/* note: Could support: M24C01, M24C02, M24C04 and M24C16 if the blocks and
   HW address are correctly defined*/
#define sEE_M24C64_32  /* Support the devices: M24C32 and M24C64 */
#endif

#ifdef sEE_M24C64_32
/* For M24C32 and M24C64 devices, E0,E1 and E2 pins are all used for device
  address selection (ne need for additional address lines). According to the
  Harware connection on the board (on STM324xG-EVAL board E0 = E1 = E2 = 0) */

#define sEE_HW_ADDRESS         0xA0   /* E0 = E1 = E2 = 0 */

#elif defined (sEE_M24C08)
/* The M24C08W contains 4 blocks (128byte each) with the adresses below: E2 = 0
   EEPROM Addresses defines */
 #define sEE_Block0_ADDRESS     0x68   /* E2 = 0 */
 /*#define sEE_Block1_ADDRESS     0xA2*/ /* E2 = 0 */
 /*#define sEE_Block2_ADDRESS     0xA4*/ /* E2 = 0 */
 /*#define sEE_Block3_ADDRESS     0xA6*/ /* E2 = 0 */

#endif /* sEE_M24C64_32 */

/* I2C clock speed configuration (in Hz)
  WARNING:
   Make sure that this define is not already declared in other files (ie.
  stm324xg_eval.h file). It can be used in parallel by other modules. */
#ifndef I2C_SPEED
#define I2C_SPEED                        100000
#endif /* I2C_SPEED */

#define I2C_SLAVE_ADDRESS7      0xA0

#if defined (sEE_M24C08)
#define sEE_PAGESIZE           16
#elif defined (sEE_M24C64_32)
#define sEE_PAGESIZE           32
#endif

/* Maximum Timeout values for flags and events waiting loops. These timeouts are
   not based on accurate values, they just guarantee that the application will
   not remain stuck if the I2C communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */
#define sEE_FLAG_TIMEOUT         ((uint32_t)0x1000)
#define sEE_LONG_TIMEOUT         ((uint32_t)(30 * sEE_FLAG_TIMEOUT))

/* Maximum number of trials for sEE_WaitEepromStandbyState() function */
#define sEE_MAX_TRIALS_NUMBER     300

/* Defintions for the state of the DMA transfer */
#define sEE_STATE_READY           0
#define sEE_STATE_BUSY            1
#define sEE_STATE_ERROR           2

#define sEE_OK                    0
#define sEE_FAIL                  1

/**
  * @}
  */

/** @defgroup STM324xG_EVAL_I2C_EE_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup STM324xG_EVAL_I2C_EE_Exported_Functions
  * @{
  */
void     sEE_DeInit(void);
void     sEE_Init(void);
uint32_t sEE_ReadBuffer(uint8_t* pBuffer, uint16_t ReadAddr, uint16_t* NumByteToRead);
uint32_t sEE_WritePage(uint8_t* pBuffer, uint16_t WriteAddr, uint8_t* NumByteToWrite);
void     sEE_WriteBuffer(uint8_t* pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite);
uint32_t sEE_WaitEepromStandbyState(void);

/* USER Callbacks: These are functions for which prototypes only are declared in
   EEPROM driver and that should be implemented into user applicaiton. */
/* sEE_TIMEOUT_UserCallback() function is called whenever a timeout condition
   occure during communication (waiting on an event that doesn't occur, bus
   errors, busy devices ...).
   You can use the default timeout callback implementation by uncommenting the
   define USE_DEFAULT_TIMEOUT_CALLBACK in stm324xg_eval_i2c_ee.h file.
   Typically the user implementation of this callback should reset I2C peripheral
   and re-initialize communication or in worst case reset all the application. */
uint32_t sEE_TIMEOUT_UserCallback(void);

#endif

#endif //F401_BALANCE_CAR_BOOTLOADER_MCU_IIC2_H
