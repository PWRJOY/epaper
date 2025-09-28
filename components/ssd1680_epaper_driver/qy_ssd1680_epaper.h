// 奇耘2.9寸墨水屏驱动
#ifndef __QY_SSD1680_EPAPER_H
#define __QY_SSD1680_EPAPER_H

#include <stdio.h> 
#include "driver/gpio.h"
#include "sdkconfig.h"   
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define QY_SSD1680_GPIO_BUSY   35      
#define QY_SSD1680_GPIO_RST    22      
#define QY_SSD1680_GPIO_DC     23      
#define QY_SSD1680_GPIO_CS     21      
#define QY_SSD1680_GPIO_SCL    18
#define QY_SSD1680_GPIO_SDA    19  

#define QY_SSD1680_EPD_SCL_Clr() 	gpio_set_level(QY_SSD1680_GPIO_SCL, 0);
#define QY_SSD1680_EPD_SCL_Set() 	gpio_set_level(QY_SSD1680_GPIO_SCL, 1);
#define QY_SSD1680_EPD_SDA_Clr() 	gpio_set_level(QY_SSD1680_GPIO_SDA, 0);
#define QY_SSD1680_EPD_SDA_Set() 	gpio_set_level(QY_SSD1680_GPIO_SDA, 1);
#define QY_SSD1680_EPD_RES_Clr() 	gpio_set_level(QY_SSD1680_GPIO_RST, 0);
#define QY_SSD1680_EPD_RES_Set() 	gpio_set_level(QY_SSD1680_GPIO_RST, 1);
#define QY_SSD1680_EPD_DC_Clr() 	gpio_set_level(QY_SSD1680_GPIO_DC, 0);
#define QY_SSD1680_EPD_DC_Set() 	gpio_set_level(QY_SSD1680_GPIO_DC, 1);
#define QY_SSD1680_EPD_CS_Clr() 	gpio_set_level(QY_SSD1680_GPIO_CS, 0);  
#define QY_SSD1680_EPD_CS_Set() 	gpio_set_level(QY_SSD1680_GPIO_CS, 1);   

// 显示模式选择
#define POS     1       // 正显
#define NEG     2       // 反显
#define OFF     3       // 清除

#define EPD_WIDTH  296
#define EPD_HEIGHT 128
#define ALLSCREEN_GRAGHBYTES	EPD_WIDTH*EPD_HEIGHT/8

void QY_SSD1680_READBUSY(void);                     // 忙等待
void QY_SSD1680_WR_Bus(unsigned char TxData);       // 向总线上写一个字节
void QY_SSD1680_WR_REG(unsigned char cmd);          // 写命令
void QY_SSD1680_WR_DATA8(unsigned char data);       // 写数据

void QY_SSD1680_HW_RESET(void);                     // 硬件复位：RST引脚
void QY_SSD1680_Init(void);                         // 无灰阶初始化
void QY_SSD1680_Init_4GRAY(void);                   // 4灰阶初始化

void QY_SSD1680_Update_and_DeepSleep_Part(void);    // 局部刷新并休眠
void QY_SSD1680_Update_and_DeepSleep(void);         // 全屏刷新并休眠
void QY_SSD1680_Update_and_DeepSleep_4GRAY(void);   // 4灰阶刷新并休眠

void QY_SSD1680_Clear(void);                        // 清屏
void QY_SSD1680_Display(const unsigned char *datas);        // 显示(无灰阶)
void QY_SSD1680_Display_Part_BaseMap(const unsigned char * datas);
void QY_SSD1680_Display_Part(int h_start,int v_start,const unsigned char * datas,int PART_WIDTH,int PART_HEIGHT,unsigned char mode);
void QY_SSD1680_Display_4GRAY(const unsigned char *datas);  // 显示(4灰阶)

void test_qy_ssd1680_epaper(void);                  // 屏幕测试函数

#endif


