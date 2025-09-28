/**
 * @file epaper.h
 * @author liying (respire_ly@qq.com)
 * @brief 墨水屏硬件驱动
 * @version 1.0
 * @date 2025-04-17
 * 
 * @copyright Copyright (c) 2025  freelance
 * 
 * @par changelog:
 * | Date | Version | Author | Description |
 * | --- | --- | --- | --- |
 * | 2025-04-17 | 1.0 | liying | 墨水屏硬件驱动 |
 */

#ifndef EPAPER_H
#define EPAPER_H


#include <stdio.h> 
#include "sdkconfig.h"   
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"    
#include "driver/gpio.h" 



#define GPIO_BUSY   35      
#define GPIO_RST    22      
#define GPIO_DC     23      
#define GPIO_CS     21      
#define GPIO_SCL    18
#define GPIO_SDA    19     

#define EPD_SCL_Clr() 	gpio_set_level(GPIO_SCL, 0);
#define EPD_SCL_Set() 	gpio_set_level(GPIO_SCL, 1);
#define EPD_SDA_Clr() 	gpio_set_level(GPIO_SDA, 0);
#define EPD_SDA_Set() 	gpio_set_level(GPIO_SDA, 1);
#define EPD_RES_Clr() 	gpio_set_level(GPIO_RST, 0);
#define EPD_RES_Set() 	gpio_set_level(GPIO_RST, 1);
#define EPD_DC_Clr() 	gpio_set_level(GPIO_DC, 0);
#define EPD_DC_Set() 	gpio_set_level(GPIO_DC, 1);
#define EPD_CS_Clr() 	gpio_set_level(GPIO_CS, 0);  
#define EPD_CS_Set() 	gpio_set_level(GPIO_CS, 1);   



#define EPD_W  180   
#define EPD_H  384


#define BLACK  0x00
#define WHITE  0x01
#define YELLOW 0x02
#define RED    0x03



//================函数声明=======================

//初始化 GPIO 引脚，为后续墨水屏操作做准备
void EPD_GPIOInit(void);        

//向总线上写入一个字节的数据
void EPD_WR_Bus(uint8_t dat);   

//向墨水屏写入指令-Command
void EPD_WR_REG(uint8_t reg);   

//向墨水屏写入 8 位数据-Parameter
void EPD_WR_DATA8(uint8_t dat);

//读取并等待墨水屏的忙碌状态
void EPD_READBUSY(void);

//对墨水屏进行硬件复位操作
void EPD_HW_RESET(void);

//更新墨水屏显示内容
void EPD_Update(void);

//使墨水屏进入深度睡眠模式以节省功耗
void EPD_DeepSleep(void);

//初始化墨水屏，设置必要的参数和状态
void EPD_Init(void);

//进行颜色转换，将输入颜色转换为特定格式
uint8_t Color_Conversion(uint8_t color);

//用指定数据填充墨水屏整个显示区域
void EPD_Display_Fill(uint8_t dat);

//在墨水屏上显示画布内容
void EPD_Display(const uint8_t *image);

#endif
