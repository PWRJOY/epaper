/**
 * @file epaper.c
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

#include "epaper.h"   

/**
 * @brief 函数功能：初始化 GPIO 引脚，为后续墨水屏操作做准备
 * @details 函数实现：BUSY引脚是输入引脚，其余均为输出引脚
 */
void EPD_GPIOInit()
{  
    gpio_config_t io_conf = {};

    io_conf.pin_bit_mask =  1ULL << GPIO_BUSY;
    io_conf.mode = GPIO_MODE_INPUT;
    gpio_config(&io_conf);

    io_conf.pin_bit_mask =  (1ULL << GPIO_RST) | (1ULL << GPIO_DC) | (1ULL << GPIO_CS) | (1ULL << GPIO_SCL) | (1ULL << GPIO_SDA);
    io_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_conf);

}    

/**
 * @brief 函数功能：向总线上写入一个字节的数据
 * @details 函数实现：首先拉低CS，从最高位开始逐个发送8位数据，墨水屏采集数据的时机是SCL上升沿，最后拉高CS
 * 
 * @param dat 需要写入的字节数据
 */
void EPD_WR_Bus(uint8_t dat)
{
	uint8_t i;

	EPD_CS_Clr();           // 拉低CS

	for(i=0;i<8;i++){       // 1个字节的8个位，从最高位开始传输
		EPD_SCL_Clr();        // 时钟SCL拉低
		if(dat&0x80){         // 显示数据SDA
			EPD_SDA_Set();      
		}else{
			EPD_SDA_Clr();
		}
		EPD_SCL_Set();        // 制造SCL上升沿
		dat<<=1;
	}

	EPD_CS_Set();	          // 拉高CS
}

/**
 * @brief 函数功能：向墨水屏写入指令-Command
 * @details 函数实现：写指令，DC线要置低，向总线发送1个字节的指令后，DC再置高
 * 
 * @param reg 写入的指令Command
 */
void EPD_WR_REG(uint8_t reg)
{
	EPD_DC_Clr();
	EPD_WR_Bus(reg);
	EPD_DC_Set();
}

/**
 * @brief 函数功能：向墨水屏写入 8 位数据-Parameter
 * @details 函数实现：写数据，DC线全程置高，向总线发送1个字节的数据
 * 
 * @param reg 写入的指令-Parameter
 */
void EPD_WR_DATA8(uint8_t dat)
{
	EPD_DC_Set();
	EPD_WR_Bus(dat);
	EPD_DC_Set();       // 其实这句是可以省略的，但是对应上面写指令的代码，没有去掉
}

/*
// 在尝试使用esp_rom_delay_us和任务看门狗的时候引入的头文件
#include "esp_rom_sys.h"
#include "esp_task_wdt.h"
*/

/**
 * @brief 函数功能：读取并等待墨水屏的忙碌状态
 * @details 函数实现：读取BUSY线的状态，BUSY为低电平代表忙碌，使用vTaskDelay非阻塞等待
 */
void EPD_READBUSY(void)
{
  while(1)
  {
    if(gpio_get_level(GPIO_BUSY)==1)
    {
      break;
    }
    //esp_rom_delay_us(5);    //报错，任务看门狗超时，不过是可以正常显示，运行程序的，每隔5秒报错任务看门狗（空闲线程的）超时，但是系统不会重启。

    //esp_rom_delay_us(5);
    //esp_task_wdt_reset();   // 不可以，喂的是自家的狗，在这里阻塞喂狗了，其他线程的看门狗还饿着
  
    //vTaskDelay(1 / portTICK_PERIOD_MS);   //1ms~5ms仍然是上面的现象  

    vTaskDelay(10 / portTICK_PERIOD_MS);    //10ms就正常了
  }
}

/**
 * @brief 函数功能：对墨水屏进行硬件复位操作
 * @details 函数实现：RES拉低一段时间进行复位，然后拉高，此操作后需检测BUSY
 */
void EPD_HW_RESET(void)
{
  vTaskDelay(100 / portTICK_PERIOD_MS);
  EPD_RES_Clr();
  vTaskDelay(10 / portTICK_PERIOD_MS);
  EPD_RES_Set();
  vTaskDelay(10 / portTICK_PERIOD_MS);
  EPD_READBUSY();
}

/**
 * @brief 函数功能：更新墨水屏显示内容
 * @details 函数实现：\n
 * 1、Power ON，R04  \n
 * 2、Dispaly Refresh,R12=0x00
 */
void EPD_Update(void)
{
  EPD_WR_REG(0x04);   //Power ON，R04
  EPD_READBUSY();

  EPD_WR_REG(0x12);   //Dispaly Refresh，R12=0x00
  EPD_WR_DATA8(0x00);
  EPD_READBUSY();
}

/**
 * @brief 函数功能：使墨水屏进入深度睡眠模式以节省功耗
 * @details 函数实现：\n 
 * 1、Power OFF，R02=0x00 \n
 * 2、Deep Sleep，R07=0xa5
 */
void EPD_DeepSleep(void)
{
  EPD_WR_REG(0x02);     //Power OFF，R02=0x00
  EPD_WR_DATA8(0x00);
  EPD_READBUSY();

  EPD_WR_REG(0x07);     //Deep Sleep，R07=0xa5
  EPD_WR_DATA8(0xa5);
}

/**
 * @brief 函数功能：初始化墨水屏，设置必要的参数和状态
 * @details 函数实现：一系列初始化墨水屏的指令和参数，需参考屏幕手册
 */
void EPD_Init(void)
{
  EPD_GPIOInit();           // 墨水屏引脚初始化

  EPD_HW_RESET();           // 复位
  
  EPD_WR_REG(0x4D);         // Enter PWD,使用PWD前需要设置，R4D=0x78
  EPD_WR_DATA8(0x78);

  EPD_WR_REG(0x00);         // 面板设置寄存器的默认值，R00=0x0F,0x09
  EPD_WR_DATA8(0x0F);
  EPD_WR_DATA8(0x09);
  
  EPD_WR_REG(0x01);         // R01H (PWR): Power Setting Register
  EPD_WR_DATA8(0x07);
  EPD_WR_DATA8(0x00);       // VGP=20V,VGN=-20V     VGP、VGN都是墨水屏的引脚
  EPD_WR_DATA8(0x22);       // VSPL_0,  0x22 = 6.4V
  EPD_WR_DATA8(0x78);       // VSP_1,   0x78 = 15V
  EPD_WR_DATA8(0x0A);       // VSN_1,   0x0A = -4V
  EPD_WR_DATA8(0x22);       // VSPL_1,  0x22 = 6.4V

  
  EPD_WR_REG(0x03);         // R03H (PFS): Power off Sequence Setting Register
  EPD_WR_DATA8(0x10);       // T_VDPG_OFF：60ms,  T_VDS_OFF=20ms
  EPD_WR_DATA8(0x54);       // VGP_EXT:2000ms,  VGP_LEN:2500ms
  EPD_WR_DATA8(0x44);        
  
  EPD_WR_REG(0x06);         // R06H (BTST): Booster Soft Start Command
  EPD_WR_DATA8(0x0F);       // PHA_SFT:40ms， PHB_SFT=40ms
  EPD_WR_DATA8(0x0A);
  EPD_WR_DATA8(0x2F);
  EPD_WR_DATA8(0x25);
  EPD_WR_DATA8(0x22);
  EPD_WR_DATA8(0x2E);
  EPD_WR_DATA8(0x21);

  EPD_WR_REG(0x30);         // R30H (PLL): PLL Control Register
  EPD_WR_DATA8(0x02);       // 禁用动态帧率，帧率设置为50Hz
  
  EPD_WR_REG(0x41);         // R41H (TSE): Temperature Sensor Calibration Register 温度传感器校准
  EPD_WR_DATA8(0x00);       // 使用内部温度传感器，校准值为默认0

  EPD_WR_REG(0x50);         // R50H (CDI): VCOM and DATA Interval Setting Register
  EPD_WR_DATA8(0x37);
  
  EPD_WR_REG(0x60);         // R60
  EPD_WR_DATA8(0x02);
  EPD_WR_DATA8(0x02);
  
  EPD_WR_REG(0x61);         // R61H (TRES): Resolution Setting，设置屏幕分辨率
  EPD_WR_DATA8(EPD_W/256);
  EPD_WR_DATA8(EPD_W%256);
  EPD_WR_DATA8(EPD_H/256);
  EPD_WR_DATA8(EPD_H%256);

  EPD_WR_REG(0x65);         // R65H (GSST): Gate/Source Start Setting Register
  EPD_WR_DATA8(0x00);
  EPD_WR_DATA8(0x00);
  EPD_WR_DATA8(0x00);
  EPD_WR_DATA8(0x00);

  EPD_WR_REG(0xE7);         // RE7
  EPD_WR_DATA8(0x1C);
  
  EPD_WR_REG(0xE3);         // RE3H (PWS): Power Saving Register 省电配置
  EPD_WR_DATA8(0x22);
  
  EPD_WR_REG(0xE0);         // RE0
  EPD_WR_DATA8(0x00);
  
  EPD_WR_REG(0xB4);         // RB4
  EPD_WR_DATA8(0xD0);
  EPD_WR_REG(0xB5);         // RB5
  EPD_WR_DATA8(0x03);
  
  EPD_WR_REG(0xE9);         // RE9
  EPD_WR_DATA8(0x01);

}

/**
 * @brief 函数功能：用指定数据填充墨水屏整个显示区域
 * 
 * @param dat 填充颜色
 */
void EPD_Display_Fill(uint8_t dat)
{
  uint16_t i;

  EPD_WR_REG(0x10);       // 开始填充数据到SRAM

  switch(dat)             // 设置填充颜色
  {
    case 0x00:
      dat=0x00;           //BLACK， 0x0000 0000  
    break;
    case 0x01:
      dat=0x55;           //WHITE， 0x0101 0101
    break;
    case 0x02:
      dat=0xAA;           //YELLOW，0x1010 1010
    break;
    case 0x03:
      dat=0xFF;           //RED，   0x1111 1111
    break;
    default:
    break;
  }  
  for(i=0;i<(EPD_W*EPD_H/4);i++)  // 一个字节可以传输4个像素的内容
  {
    EPD_WR_DATA8(dat);
  }  
}

/**
 * @brief 函数功能：进行颜色转换，将输入颜色转换为特定格式
 * @details 函数实现：没必要，在定义宏的时候其实已经相当于转换了
 * 
 * @param color 颜色
 * @return uint8_t 转换后的颜色
 */
uint8_t Color_Conversion(uint8_t color)
{
  uint8_t datas = 0;
  switch(color)
  {
    case 0x00:
      datas=BLACK;  
      break;    
    case 0x01:
      datas=WHITE; 
      break;
    case 0x02:
      datas=YELLOW; 
      break;    
    case 0x03:
      datas=RED; 
      break;      
    default:
      break;      
  }
   return datas;
}

/**
 * @brief 函数功能：在墨水屏上显示画布内容
 * 
 * @param image 画布数据
 */
void EPD_Display(const uint8_t *image)
{
  uint8_t data_H1,data_H2,data_L1,data_L2,data,temp;
  uint16_t i,j,Width,Height;

  Width=(EPD_W%4==0)?(EPD_W/4):(EPD_W/4+1); // EPD_W=180，180/4=45
  Height=EPD_H;

  EPD_WR_REG(0x10);                   // 开始写入数据到SRAM的指令

  for (j=0;j<Height;j++) 
  {
    for (i=0;i<Width;i++) 
    {
      temp=image[i+j*Width];          // 取出image的某一个字节数据，8位，每2位代表一个像素的颜色，刚好4个像素
      data_H1=Color_Conversion(temp>>6&0x03)<<6;      
      data_H2=Color_Conversion(temp>>4&0x03)<<4;
      data_L1=Color_Conversion(temp>>2&0x03)<<2;
      data_L2=Color_Conversion(temp&0x03);
      data=data_H1|data_H2|data_L1|data_L2;
      EPD_WR_DATA8(data);             // 发送4个像素的内容
    }
  }
}




