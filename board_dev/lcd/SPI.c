#include "spi.h"
#include "mcu_spi.h"

/*****************************************************************************
 * @name       :u8 SPI_WriteByte(SPI_TypeDef* SPIx,u8 Byte)
 * @date       :2018-08-09
 * @function   :Write a byte of data using STM32's hardware SPI
 * @parameters :SPIx: SPI type,x for 1,2,3
                Byte:Data to be written
 * @retvalue   :Data received by the bus
******************************************************************************/


/*****************************************************************************
 * @name       :void SPI_SetSpeed(SPI_TypeDef* SPIx,u8 SpeedSet)
 * @date       :2018-08-09
 * @function   :Set hardware SPI Speed
 * @parameters :SPIx: SPI type,x for 1,2,3
                SpeedSet:0-high speed
												 1-low speed
 * @retvalue   :None
******************************************************************************/
void SPI_SetSpeed(SPI_TypeDef *SPIx, u8 SpeedSet) {

}

/*****************************************************************************
 * @name       :void SPI1_Init(void)
 * @date       :2018-08-09
 * @function   :Initialize the STM32 hardware SPI1
 * @parameters :None
 * @retvalue   :None
******************************************************************************/


void SPI2_Send_framebuffer_init_to_16bit(uint8_t is16bit) {

    /* USER CODE BEGIN SPI2_Init 0 */

    /* USER CODE END SPI2_Init 0 */

    /* USER CODE BEGIN SPI2_Init 1 */

    /* USER CODE END SPI2_Init 1 */
    SPI2->CR1 &= ~(1 << 6);  //关闭SPI2
    if (is16bit > 0) {
        SPI2->CR1 |= (1 << 11);   //16 bit
    } else {
        SPI2->CR1 &= ~(1 << 11);
    }

    SPI2->CR1 |= (1 << 6);  //开启SPI2
}


