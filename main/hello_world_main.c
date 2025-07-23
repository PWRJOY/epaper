/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "../components/epaper_driver/epaper.h"
#include "../components/epaper_driver/epaper_gui.h"
#include "esp_task_wdt.h"
#include "esp_rom_sys.h"

// 画布像素数据
uint8_t ImageBW[EPD_W*EPD_H/4];


void app_main(void)
{
    printf("Hello world!20250720-16.59\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
/*
    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
*/

    //esp_task_wdt_add(NULL);




  /***********************墨水屏************************/
  EPD_Init();                                   // 墨水屏初始化
  Paint_NewImage(ImageBW,EPD_W,EPD_H,0,WHITE);  // 创建画布，画布数据存放于数组ImageBW

  /*
    //一些墨水屏的测试函数
  Paint_Clear(WHITE);                           // 画布清屏
  EPD_ShowString(80,0,(uint8_t*)"Welcome to 3.52-inch E-paper",16,BLACK,WHITE);   // 显示字符串
  EPD_ShowString(90,20,(uint8_t*)"with 384 x 180 resolution",16,RED,WHITE);       // 显示字符串
  EPD_ShowString(110,40,(uint8_t*)"Demo-Test-2023/10/16",16,YELLOW,WHITE);        // 显示字符串
  
  EPD_DrawLine(0,60,EPD_H-1,60,BLACK);          // 画线
  EPD_DrawRectangle(24,78,64,118,BLACK,0);      // 画矩形
  EPD_DrawRectangle(98,78,138,119,BLACK,1);     // 画矩形
  EPD_ShowWatch(138,74,12.05,4,2,48,RED,WHITE); // 显示时间
  EPD_DrawCircle(295,100,25,BLACK,0);           // 画圆
  EPD_DrawCircle(345,100,25,BLACK,1);           // 画圆
  */

  //EPD_ShowChinese(0,74,(uint8_t*)"中景园电子",16,BLACK);  //显示汉字
  Paint_Clear(WHITE);  
  EPD_DrawLine(0,60,EPD_H-1,60,YELLOW);          // 画线  
  EPD_Display(ImageBW);                         // 将画布内容发送到SRAM
  EPD_Update();                                 // 刷新SRAM内容显示到墨水屏

  while(1){
    vTaskDelay(1000 / portTICK_PERIOD_MS);      // 非阻塞延时，用于任务切换
    //esp_task_wdt_reset();
  }

}
