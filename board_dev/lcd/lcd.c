
#include "lcd.h"
#include "stdlib.h"
#include "main.h"
#include "mcu_spi.h"


//管理LCD重要参数
//默认为竖屏
_lcd_dev lcddev;

//画笔颜色,背景颜色
u16 POINT_COLOR = 0x0000, BACK_COLOR = 0xFFFF;
u16 DeviceCode;

__STATIC_FORCEINLINE void SPI1_Init(void) {
    mcu_spi_init();
}



/**
 * 使用了 __STATIC_FORCEINLINE 修饰之后，运行效率和 #define  SPI_WriteByte(a, b) mcu_spi2_send_byte_ll(b)完全一致
 * 此函数，调用的函数也是__STATIC_FORCEINLINE函数，实际测试发现，套1层和套2层，效率是一样的
 * @param Byte
 */
__STATIC_FORCEINLINE void SPI_WriteByte(SPI_TypeDef *SPIx, u8 Byte) {
    mcu_spi2_send_byte_ll(Byte);
}

/*****************************************************************************
 * @name       :void LCD_WR_REG(u8 data)
 * @date       :2018-08-09 
 * @function   :Write an 8-bit command to the LCD screen
 * @parameters :data:Command value to be written
 * @retvalue   :None
******************************************************************************/
void LCD_WR_REG(u8 data) {
    LCD_CS_CLR;
    LCD_RS_CLR;
    SPI_WriteByte(SPI1, data);
    LCD_CS_SET;
}

/*****************************************************************************
 * @name       :void LCD_WR_DATA(u8 data)
 * @date       :2018-08-09
 * @function   :Write an 8-bit data to the LCD screen
 * @parameters :data:data value to be written
 * @retvalue   :None
******************************************************************************/
void LCD_WR_DATA(u8 data) {
    LCD_CS_CLR;
    LCD_RS_SET;
    SPI_WriteByte(SPI1, data);
    LCD_CS_SET;
}

/*****************************************************************************
 * @name       :void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue)
 * @date       :2018-08-09
 * @function   :Write data into registers
 * @parameters :LCD_Reg:Register address
                LCD_RegValue:Data to be written
 * @retvalue   :None
******************************************************************************/
void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue) {
    LCD_WR_REG(LCD_Reg);
    LCD_WR_DATA(LCD_RegValue);
}

/*****************************************************************************
 * @name       :void LCD_WriteRAM_Prepare(void)
 * @date       :2018-08-09
 * @function   :Write GRAM
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void LCD_WriteRAM_Prepare(void) {
    LCD_WR_REG(lcddev.wramcmd);
}

/*****************************************************************************
 * @name       :void Lcd_WriteData_16Bit(u16 Data)
 * @date       :2018-08-09
 * @function   :Write an 16-bit command to the LCD screen
 * @parameters :Data:Data to be written
 * @retvalue   :None
******************************************************************************/
void Lcd_WriteData_16Bit(u16 Data) {
    LCD_CS_CLR;
    LCD_RS_SET;
    SPI2_set_dataformat_16bit();
  //  SPI2_set_dataformat_init_to_16bit(1);
    mcu_spi2_send_16bit_ll(Data);
  //  SPI2_set_dataformat_init_to_16bit(0);
    SPI2_set_dataformat_8bit();
    LCD_CS_SET;
}

/*****************************************************************************
 * @name       :void LCD_DrawPoint(u16 x,u16 y)
 * @date       :2018-08-09
 * @function   :Write a pixel data at a specified location
 * @parameters :x:the x coordinate of the pixel
                y:the y coordinate of the pixel
 * @retvalue   :None
******************************************************************************/
void LCD_DrawPoint(u16 x, u16 y) {
    LCD_SetCursor(x, y);//设置光标位置
    Lcd_WriteData_16Bit(POINT_COLOR);
}

/*****************************************************************************
 * @name       :void LCD_Clear(u16 Color)
 * @date       :2018-08-09
 * @function   :Full screen filled LCD screen
 * @parameters :color:Filled color
 * @retvalue   :None
******************************************************************************/
void LCD_Clear(u16 Color) {
    unsigned int i, m;
    LCD_SetWindows(0, 0, lcddev.width - 1, lcddev.height - 1);
    LCD_CS_CLR;
    LCD_RS_SET;
    SPI2_set_dataformat_16bit();
    for (i = 0; i < lcddev.height; i++) {
        for (m = 0; m < lcddev.width; m++) {

            mcu_spi2_send_16bit_ll(Color);

//            SPI_WriteByte(SPI1, Color >> 8);
//            SPI_WriteByte(SPI1, Color);
        }
    }
    SPI2_set_dataformat_8bit();

    LCD_CS_SET;
}

/*****************************************************************************
 * @name       :void LCD_Clear(u16 Color)
 * @date       :2018-08-09 
 * @function   :Initialization LCD screen GPIO
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void LCD_GPIOInit(void) {
    //SPI2-> PB15 MOSI,PB14 MISO, PB13 SCK, PB12 NRF24CS, PB9 LCD-CS
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    //初始化片选，
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = LCD_BLK_Pin;
    GPIO_Init(LCD_BLK_GPIO_Port, &GPIO_InitStructure);
    GPIO_SetBits(LCD_BLK_GPIO_Port, LCD_BLK_Pin);

    GPIO_InitStructure.GPIO_Pin = LCD_CS_Pin;
    GPIO_Init(LCD_CS_GPIO_Port, &GPIO_InitStructure);
    GPIO_SetBits(LCD_CS_GPIO_Port, LCD_CS_Pin);

    GPIO_InitStructure.GPIO_Pin = LCD_RESET_Pin;
    GPIO_Init(LCD_RESET_GPIO_Port, &GPIO_InitStructure);
    GPIO_SetBits(LCD_RESET_GPIO_Port, LCD_RESET_Pin);

    GPIO_InitStructure.GPIO_Pin = LCD_DC_Pin;
    GPIO_Init(LCD_DC_GPIO_Port, &GPIO_InitStructure);
    GPIO_SetBits(LCD_DC_GPIO_Port, LCD_DC_Pin);
}

/*****************************************************************************
 * @name       :void LCD_RESET(void)
 * @date       :2018-08-09 
 * @function   :Reset LCD screen
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void LCD_RESET(void) {
    LCD_RST_CLR;
    vTaskDelay(20);
    LCD_RST_SET;
    vTaskDelay(20);
}

/*****************************************************************************
 * @name       :void LCD_RESET(void)
 * @date       :2018-08-09 
 * @function   :Initialization LCD screen
 * @parameters :None
 * @retvalue   :None
******************************************************************************/
void LCD_Init(void) {
    SPI1_Init(); //硬件SPI初始化
//	SPI_SetSpeed(SPI1,SPI_BaudRatePrescaler_2);
    LCD_GPIOInit();//LCD GPIO初始化										 
    LCD_RESET(); //LCD 复位
//************* ST7789初始化**********//	
    LCD_WR_REG(0x36);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0x3A);
    LCD_WR_DATA(0x05);

    LCD_WR_REG(0xB2);
    LCD_WR_DATA(0x0C);
    LCD_WR_DATA(0x0C);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x33);
    LCD_WR_DATA(0x33);

    LCD_WR_REG(0xB7);
    LCD_WR_DATA(0x35);

    LCD_WR_REG(0xBB);
    LCD_WR_DATA(0x19);

    LCD_WR_REG(0xC0);
    LCD_WR_DATA(0x2C);

    LCD_WR_REG(0xC2);
    LCD_WR_DATA(0x01);

    LCD_WR_REG(0xC3);
    LCD_WR_DATA(0x12);

    LCD_WR_REG(0xC4);
    LCD_WR_DATA(0x20);

    LCD_WR_REG(0xC6);
    LCD_WR_DATA(0x0F);

    LCD_WR_REG(0xD0);
    LCD_WR_DATA(0xA4);
    LCD_WR_DATA(0xA1);

    LCD_WR_REG(0xE0);
    LCD_WR_DATA(0xD0);
    LCD_WR_DATA(0x04);
    LCD_WR_DATA(0x0D);
    LCD_WR_DATA(0x11);
    LCD_WR_DATA(0x13);
    LCD_WR_DATA(0x2B);
    LCD_WR_DATA(0x3F);
    LCD_WR_DATA(0x54);
    LCD_WR_DATA(0x4C);
    LCD_WR_DATA(0x18);
    LCD_WR_DATA(0x0D);
    LCD_WR_DATA(0x0B);
    LCD_WR_DATA(0x1F);
    LCD_WR_DATA(0x23);

    LCD_WR_REG(0xE1);
    LCD_WR_DATA(0xD0);
    LCD_WR_DATA(0x04);
    LCD_WR_DATA(0x0C);
    LCD_WR_DATA(0x11);
    LCD_WR_DATA(0x13);
    LCD_WR_DATA(0x2C);
    LCD_WR_DATA(0x3F);
    LCD_WR_DATA(0x44);
    LCD_WR_DATA(0x51);
    LCD_WR_DATA(0x2F);
    LCD_WR_DATA(0x1F);
    LCD_WR_DATA(0x1F);
    LCD_WR_DATA(0x20);
    LCD_WR_DATA(0x23);

    LCD_WR_REG(0x21);

    LCD_WR_REG(0x11);
    //Delay (120); 

    LCD_WR_REG(0x29);
    LCD_direction(USE_HORIZONTAL);//设置LCD显示方向
    LCD_LED = 1;//点亮背光
    LCD_Clear(WHITE);//清全屏白色
}

/*****************************************************************************
 * @name       :void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd)
 * @date       :2018-08-09 
 * @function   :Setting LCD display window
 * @parameters :xStar:the bebinning x coordinate of the LCD display window
								yStar:the bebinning y coordinate of the LCD display window
								xEnd:the endning x coordinate of the LCD display window
								yEnd:the endning y coordinate of the LCD display window
 * @retvalue   :None
******************************************************************************/
void LCD_SetWindows(u16 xStar, u16 yStar, u16 xEnd, u16 yEnd) {
    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA((xStar + lcddev.xoffset) >> 8);
    LCD_WR_DATA(xStar + lcddev.xoffset);
    LCD_WR_DATA((xEnd + lcddev.xoffset) >> 8);
    LCD_WR_DATA(xEnd + lcddev.xoffset);

    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA((yStar + lcddev.yoffset) >> 8);
    LCD_WR_DATA(yStar + lcddev.yoffset);
    LCD_WR_DATA((yEnd + lcddev.yoffset) >> 8);
    LCD_WR_DATA(yEnd + lcddev.yoffset);

    LCD_WriteRAM_Prepare();    //开始写入GRAM
}

/*****************************************************************************
 * @name       :void LCD_SetCursor(u16 Xpos, u16 Ypos)
 * @date       :2018-08-09 
 * @function   :Set coordinate value
 * @parameters :Xpos:the  x coordinate of the pixel
								Ypos:the  y coordinate of the pixel
 * @retvalue   :None
******************************************************************************/
void LCD_SetCursor(u16 Xpos, u16 Ypos) {
    LCD_SetWindows(Xpos, Ypos, Xpos, Ypos);
}

/*****************************************************************************
 * @name       :void LCD_direction(u8 direction)
 * @date       :2018-08-09 
 * @function   :Setting the display direction of LCD screen
 * @parameters :direction:0-0 degree
                          1-90 degree
													2-180 degree
													3-270 degree
 * @retvalue   :None
******************************************************************************/
void LCD_direction(u8 direction) {
    lcddev.setxcmd = 0x2A;
    lcddev.setycmd = 0x2B;
    lcddev.wramcmd = 0x2C;
    switch (direction) {
        case 0:
            lcddev.width = LCD_W;
            lcddev.height = LCD_H;
            lcddev.xoffset = 0;
            lcddev.yoffset = 0;
            LCD_WriteReg(0x36, 0);//BGR==1,MY==0,MX==0,MV==0
            break;
        case 1:
            lcddev.width = LCD_H;
            lcddev.height = LCD_W;
            lcddev.xoffset = 0;
            lcddev.yoffset = 0;
            LCD_WriteReg(0x36, (1 << 6) | (1 << 5));//BGR==1,MY==1,MX==0,MV==1
            break;
        case 2:
            lcddev.width = LCD_W;
            lcddev.height = LCD_H;
            lcddev.xoffset = 0;
            lcddev.yoffset = 80;
            LCD_WriteReg(0x36, (1 << 6) | (1 << 7));//BGR==1,MY==0,MX==0,MV==0
            break;
        case 3:
            lcddev.width = LCD_H;
            lcddev.height = LCD_W;
            lcddev.xoffset = 80;
            lcddev.yoffset = 0;
            LCD_WriteReg(0x36, (1 << 7) | (1 << 5));//BGR==1,MY==1,MX==0,MV==1
            break;
        default:
            break;
    }
}	 
