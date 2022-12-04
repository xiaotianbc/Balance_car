//
// Created by xiaotian on 2022/11/29.
//

#include "board_spi2.h"
#include "main.h"


#define SPIx_CLK_INIT RCC_APB1PeriphClockCmd
#define SPIx_CLK RCC_APB1Periph_SPI2
#define SPIx_SCK_GPIO_CLK RCC_AHB1Periph_GPIOB
#define SPIx_MISO_GPIO_CLK RCC_AHB1Periph_GPIOB
#define SPIx_MOSI_GPIO_CLK RCC_AHB1Periph_GPIOB

#define SPIx_SCK_GPIO_PORT GPIOB
#define SPIx_MISO_GPIO_PORT GPIOB
#define SPIx_MOSI_GPIO_PORT GPIOB

#define SPIx_SCK_SOURCE GPIO_PinSource13
#define SPIx_MISO_SOURCE GPIO_PinSource14
#define SPIx_MOSI_SOURCE GPIO_PinSource15

#define  SPIx_SCK_AF GPIO_AF_SPI2
#define  SPIx_MISO_AF GPIO_AF_SPI2
#define  SPIx_MOSI_AF GPIO_AF_SPI2

#define  SPIx_SCK_PIN GPIO_Pin_13
#define  SPIx_MISO_PIN GPIO_Pin_14
#define  SPIx_MOSI_PIN GPIO_Pin_15

#define SPIx SPI2
#define SPIx_IRQn SPI2_IRQn


/*DMA配置*/
// SPI2_TX: DMA1_Stream 4 Channel 0
// SPI2_RX: DMA1_Stream 3 Channel 0

#define SPIx_DMA_CLK RCC_AHB1Periph_DMA1
#define  SPIx_TX_DMA_STREAM  DMA1_Stream4
#define  SPIx_RX_DMA_STREAM  DMA1_Stream3
#define SPIx_TX_DMA_CHANNEL DMA_Channel_0
#define SPIx_RX_DMA_CHANNEL DMA_Channel_0
#define  BUFFERSIZE 1024


#define aTxBuffer 0x12345678
#define aRxBuffer 0x87654321

void board_spi2_init(void){
    GPIO_InitTypeDef GPIO_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;
    /* Peripheral Clock Enable -------------------------------------------------*/
    /* Enable the SPI clock */
    SPIx_CLK_INIT(SPIx_CLK, ENABLE);

    /* Enable GPIO clocks */
    RCC_AHB1PeriphClockCmd(SPIx_SCK_GPIO_CLK | SPIx_MISO_GPIO_CLK | SPIx_MOSI_GPIO_CLK, ENABLE);

    /* Enable DMA clock */
    RCC_AHB1PeriphClockCmd(SPIx_DMA_CLK, ENABLE);

    /* SPI GPIO Configuration --------------------------------------------------*/
    /* GPIO Deinitialisation */
    GPIO_DeInit(SPIx_SCK_GPIO_PORT);
    GPIO_DeInit(SPIx_MISO_GPIO_PORT);
    GPIO_DeInit(SPIx_MOSI_GPIO_PORT);

    /* Connect SPI pins to AF5 */
    GPIO_PinAFConfig(SPIx_SCK_GPIO_PORT, SPIx_SCK_SOURCE, SPIx_SCK_AF);
    GPIO_PinAFConfig(SPIx_MISO_GPIO_PORT, SPIx_MISO_SOURCE, SPIx_MISO_AF);
    GPIO_PinAFConfig(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_SOURCE, SPIx_MOSI_AF);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

    /* SPI SCK pin configuration */
    GPIO_InitStructure.GPIO_Pin = SPIx_SCK_PIN;
    GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStructure);

    /* SPI  MISO pin configuration */
    GPIO_InitStructure.GPIO_Pin =  SPIx_MISO_PIN;
    GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStructure);

    /* SPI  MOSI pin configuration */
    GPIO_InitStructure.GPIO_Pin =  SPIx_MOSI_PIN;
    GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStructure);

    /* SPI configuration -------------------------------------------------------*/
    SPI_I2S_DeInit(SPIx);
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;

    /* DMA configuration -------------------------------------------------------*/
    /* Deinitialize DMA Streams */
    DMA_DeInit(SPIx_TX_DMA_STREAM);
    DMA_DeInit(SPIx_RX_DMA_STREAM);

    /* Configure DMA Initialization Structure */
    DMA_InitStructure.DMA_BufferSize = BUFFERSIZE ;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t) (&(SPIx->DR)) ;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    /* Configure TX DMA */
    DMA_InitStructure.DMA_Channel = SPIx_TX_DMA_CHANNEL ;
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral ;
    DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)aTxBuffer ;
    DMA_Init(SPIx_TX_DMA_STREAM, &DMA_InitStructure);
    /* Configure RX DMA */
    DMA_InitStructure.DMA_Channel = SPIx_RX_DMA_CHANNEL ;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;
    DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)aRxBuffer ;
    DMA_Init(SPIx_RX_DMA_STREAM, &DMA_InitStructure);

}