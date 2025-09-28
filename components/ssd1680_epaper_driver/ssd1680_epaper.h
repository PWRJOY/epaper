// 中景园2.9寸墨水屏
#ifndef __SSD1680_EPAPER_H
#define __SSD1680_EPAPER_H

#include <stdio.h> 
#include "driver/gpio.h"
#include "sdkconfig.h"   
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// GPIO 定义
#define SSD1680_GPIO_BUSY   35      
#define SSD1680_GPIO_RST    22      
#define SSD1680_GPIO_DC     23      
#define SSD1680_GPIO_CS     21      
#define SSD1680_GPIO_SCL    18
#define SSD1680_GPIO_SDA    19  


#define SSD1680_EPD_SCL_Clr() 	gpio_set_level(SSD1680_GPIO_SCL, 0);
#define SSD1680_EPD_SCL_Set() 	gpio_set_level(SSD1680_GPIO_SCL, 1);
#define SSD1680_EPD_SDA_Clr() 	gpio_set_level(SSD1680_GPIO_SDA, 0);
#define SSD1680_EPD_SDA_Set() 	gpio_set_level(SSD1680_GPIO_SDA, 1);
#define SSD1680_EPD_RES_Clr() 	gpio_set_level(SSD1680_GPIO_RST, 0);
#define SSD1680_EPD_RES_Set() 	gpio_set_level(SSD1680_GPIO_RST, 1);
#define SSD1680_EPD_DC_Clr() 	gpio_set_level(SSD1680_GPIO_DC, 0);
#define SSD1680_EPD_DC_Set() 	gpio_set_level(SSD1680_GPIO_DC, 1);
#define SSD1680_EPD_CS_Clr() 	gpio_set_level(SSD1680_GPIO_CS, 0);  
#define SSD1680_EPD_CS_Set() 	gpio_set_level(SSD1680_GPIO_CS, 1);   


#define SSD1680_W   128
#define SSD1680_H   296


typedef struct {
    uint8_t *Image;
    uint16_t Width;
    uint16_t Height;
    uint16_t WidthMemory;
    uint16_t HeightMemory;
    uint16_t Color;
    uint16_t Rotate;
    uint16_t WidthByte;
    uint16_t HeightByte;
} SSD1680_PAINT;
extern SSD1680_PAINT SSD1680_Paint;

#define SSD1680_ROTATE_0            0   
#define SSD1680_ROTATE_90           90  
#define SSD1680_ROTATE_180          180 
#define SSD1680_ROTATE_270          270 


#define SSD1680_WHITE          0xFF 
#define SSD1680_BLACK          0x00  

void SSD1680_GPIOInit(void);
void SSD1680_WR_Bus(uint8_t dat);
void SSD1680_WR_REG(uint8_t reg);
void SSD1680_WR_DATA8(uint8_t dat); 
void SSD1680_READBUSY(void);
void SSD1680_HW_RESET(void);
void SSD1680_Update(void);
void SSD1680_Init(void);
void SSD1680_Paint_NewImage(uint8_t *image,uint16_t Width,uint16_t Height,uint16_t Rotate,uint16_t Color);
void SSD1680_Paint_SetPixel(uint16_t Xpoint,uint16_t Ypoint,uint16_t Color);
void SSD1680_Clear(uint16_t Color);
void SSD1680_DrawPoint(uint16_t Xpoint,uint16_t Ypoint,uint16_t Color);
void SSD1680_DrawLine(uint16_t Xstart,uint16_t Ystart,uint16_t Xend,uint16_t Yend,uint16_t Color);
void SSD1680_DrawRectangle(uint16_t Xstart,uint16_t Ystart,uint16_t Xend,uint16_t Yend,uint16_t Color,uint8_t mode);
void SSD1680_DrawCircle(uint16_t X_Center,uint16_t Y_Center,uint16_t Radius,uint16_t Color,uint8_t mode); 
void SSD1680_ShowChar(uint16_t x,uint16_t y,uint16_t chr,uint16_t size1,uint16_t color);
void SSD1680_ShowString(uint16_t x,uint16_t y,uint8_t *chr,uint16_t size1,uint16_t color);
void SSD1680_Display(unsigned char *Image);


#endif


