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

/**
 * @defgroup epd_define_gpio 墨水屏GPIO引脚和高低电平控制
 */

/**
 * @def GPIO_BUSY
 * @brief 表示 BUSY 状态的输入 GPIO 引脚，编号为 25
 * @ingroup epd_define_gpio
 */
#define GPIO_BUSY   25      

/**
 * @def GPIO_RST
 * @brief 用于复位操作的输出 GPIO 引脚，编号为 26
 * @ingroup epd_define_gpio
 */
#define GPIO_RST    26      

/**
 * @def GPIO_DC
 * @brief 用于数据/命令选择的输出 GPIO 引脚，编号为 27
 * @ingroup epd_define_gpio
 */
#define GPIO_DC     27      

/**
 * @def GPIO_CS
 * @brief 用于片选操作的输出 GPIO 引脚，编号为 15
 * @ingroup epd_define_gpio
 */
#define GPIO_CS     15      

/**
 * @def GPIO_SCL
 * @brief 用于 SPI 时钟信号的输出 GPIO 引脚，编号为 13
 * @ingroup epd_define_gpio
 */
#define GPIO_SCL    13      //输出

/**
 * @def GPIO_SDA
 * @brief 用于 SPI 数据传输的输出 GPIO 引脚，编号为 14
 * @ingroup epd_define_gpio
 */
#define GPIO_SDA    14      

/**
 * @def EPD_SCL_Clr()
 * @brief 将 SPI 时钟信号（SCL）对应的 GPIO_SCL 引脚电平拉低
 * @ingroup epd_define_gpio
 */
#define EPD_SCL_Clr() 	gpio_set_level(GPIO_SCL, 0);

/**
 * @def EPD_SCL_Set()
 * @brief 将 SPI 时钟信号（SCL）对应的 GPIO_SCL 引脚电平拉高
 * @ingroup epd_define_gpio
 */
#define EPD_SCL_Set() 	gpio_set_level(GPIO_SCL, 1);

/**
 * @def EPD_SDA_Clr()
 * @brief 将 SPI 数据传输（SDA）对应的 GPIO_SDA 引脚电平拉低
 * @ingroup epd_define_gpio
 */
#define EPD_SDA_Clr() 	gpio_set_level(GPIO_SDA, 0);

/**
 * @def EPD_SDA_Set()
 * @brief 将 SPI 数据传输（SDA）对应的 GPIO_SDA 引脚电平拉高
 * @ingroup epd_define_gpio
 */
#define EPD_SDA_Set() 	gpio_set_level(GPIO_SDA, 1);

/**
 * @def EPD_RES_Clr()
 * @brief 将复位信号（RES）对应的 GPIO_RST 引脚电平拉低以进行复位操作
 * @ingroup epd_define_gpio
 */
#define EPD_RES_Clr() 	gpio_set_level(GPIO_RST, 0);

/**
 * @def EPD_RES_Set()
 * @brief 将复位信号（RES）对应的 GPIO_RST 引脚电平拉高
 * @ingroup epd_define_gpio
 */
#define EPD_RES_Set() 	gpio_set_level(GPIO_RST, 1);

/**
 * @def EPD_DC_Clr()
 * @brief 将数据/命令选择信号（DC）对应的 GPIO_DC 引脚电平拉低以选择命令模式
 * @ingroup epd_define_gpio
 */
#define EPD_DC_Clr() 	gpio_set_level(GPIO_DC, 0);

/**
 * @def EPD_DC_Set()
 * @brief 将数据/命令选择信号（DC）对应的 GPIO_DC 引脚电平拉高以选择数据模式
 * @ingroup epd_define_gpio
 */
#define EPD_DC_Set() 	gpio_set_level(GPIO_DC, 1);

/**
 * @def EPD_CS_Clr()
 * @brief 将片选信号（CS）对应的 GPIO_CS 引脚电平拉低以选中设备
 * @ingroup epd_define_gpio
 */
#define EPD_CS_Clr() 	gpio_set_level(GPIO_CS, 0);  

/**
 * @def EPD_CS_Set()
 * @brief 将片选信号（CS）对应的 GPIO_CS 引脚电平拉高以取消选中设备
 * @ingroup epd_define_gpio
 */
#define EPD_CS_Set() 	gpio_set_level(GPIO_CS, 1);   


/**
 * @defgroup epd_define_feather 墨水屏分辨率、颜色
 */

/**
 * @def EPD_W
 * @brief 定义墨水屏的宽度，值为 180 像素
 * @ingroup epd_define_feather
 */
#define EPD_W  180   

/**
 * @def EPD_H
 * @brief 定义墨水屏的高度，值为 384 像素
 * @ingroup epd_define_feather
 */
#define EPD_H  384

/**
 * @def BLACK
 * @brief 定义墨水屏颜色常量，黑色为 0x00
 * @ingroup epd_define_feather
 */
#define BLACK  0x00

/**
 * @def WHITE
 * @brief 定义墨水屏颜色常量，白色为 0x01
 * @ingroup epd_define_feather
 */
#define WHITE  0x01

/**
 * @def YELLOW
 * @brief 定义墨水屏颜色常量，黄色为 0x02
 * @ingroup epd_define_feather
 */
#define YELLOW 0x02

/**
 * @def RED
 * @brief 定义墨水屏颜色常量，红色为 0x03
 * @ingroup epd_define_feather
 */
#define RED    0x03



//================函数声明=======================

/** @brief 函数功能：初始化 GPIO 引脚，为后续墨水屏操作做准备 */
void EPD_GPIOInit(void);        

/** @brief 函数功能：向总线上写入一个字节的数据 */
void EPD_WR_Bus(uint8_t dat);   

/** @brief 函数功能：向墨水屏写入指令-Command */
void EPD_WR_REG(uint8_t reg);   

/** @brief 函数功能：向墨水屏写入 8 位数据-Parameter */
void EPD_WR_DATA8(uint8_t dat);

/** @brief 函数功能：读取并等待墨水屏的忙碌状态 */
void EPD_READBUSY(void);

/** @brief 函数功能：对墨水屏进行硬件复位操作 */
void EPD_HW_RESET(void);

/** @brief 函数功能：更新墨水屏显示内容 */
void EPD_Update(void);

/** @brief 函数功能：使墨水屏进入深度睡眠模式以节省功耗 */
void EPD_DeepSleep(void);

/** @brief 函数功能：初始化墨水屏，设置必要的参数和状态 */
void EPD_Init(void);

/** @brief 函数功能：进行颜色转换，将输入颜色转换为特定格式 */
uint8_t Color_Conversion(uint8_t color);

/** @brief 函数功能：用指定数据填充墨水屏整个显示区域 */
void EPD_Display_Fill(uint8_t dat);

/** @brief 函数功能：在墨水屏上显示画布内容 */
void EPD_Display(const uint8_t *image);

#endif
