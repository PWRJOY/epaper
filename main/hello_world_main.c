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
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "quote_fetcher.h"
#include "config.h"
#include "wifi.h"
#include "board_init.h"
#include <string.h>

// 画布像素数据
uint8_t ImageBW[EPD_W*EPD_H/4];

// ✅ 显示语录到墨水屏的函数
void display_quote_on_epaper(const char *quote) {
    static char last_quote[256] = {0};  // 保存上一次显示的内容

    // 如果和上次显示内容一致，则不刷新
    if (strcmp(quote, last_quote) == 0) {
        ESP_LOGI("EPD", "语录未变化，跳过刷新");
        return;
    }

    // 保存新内容
    strncpy(last_quote, quote, sizeof(last_quote) - 1);
    last_quote[sizeof(last_quote) - 1] = '\0';

    // 开始刷新
    Paint_Clear(WHITE);  
    EPD_ShowString(0, 0, (uint8_t *)quote, 16, BLACK, WHITE);  
    EPD_Display(ImageBW);  
    EPD_Update();  
}


void app_main(void)
{
    print_chip_info();          // 打印芯片信息
    init_nvs();                 // 初始化 NVS
    wifi_init_sta();            // 初始化并连接WiFi
    wait_for_wifi_connection(); // 等待WiFi连接成功

    EPD_Init();                                   // 墨水屏初始化
    Paint_NewImage(ImageBW,EPD_W,EPD_H,0,WHITE);  // 创建画布，画布数据存放于数组ImageBW


    register_quote_display_callback(display_quote_on_epaper);   // 注册显示函数
    start_quote_fetch_task();                                   // 启动语录获取任务

    while(1){
        vTaskDelay(1000 / portTICK_PERIOD_MS);      // 非阻塞延时，用于任务切换
    }

}
