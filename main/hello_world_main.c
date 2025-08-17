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

static const char *TAG = "main";

// 画布像素数据，17280字节=16.875Kb
uint8_t ImageBW[EPD_W*EPD_H/4];

// 显示语录到墨水屏的函数
void display_quote_on_epaper(const char *quote, const GlyphBitmap *glyphs, int glyph_count, const GlyphPlacement *placements) {
    static char last_quote[256] = {0};

    if (strcmp(quote, last_quote) == 0) {
        ESP_LOGI("EPD", "语录未变化，跳过刷新");
        return;
    }

    strncpy(last_quote, quote, sizeof(last_quote) - 1);
    last_quote[sizeof(last_quote) - 1] = '\0';

    Paint_Clear(WHITE);

    int char_index = 0;           // 当前处理的字符索引
    // 遍历语录中的每个字符，查找对应的位图并绘制
    for (int i = 0; quote[i] != '\0'; ) {

        // 计算当前字符的 UTF-8 长度
        int len = get_utf8_char_length(quote[i]);    
        if (len == 0) {
            ESP_LOGW("EPD", "invalid UTF-8 coding.");
            return;                               // 无效的 UTF-8 编码，直接返回
        }    

        char utf8_char[5] = {0};
        strncpy(utf8_char, &quote[i], len);

        const uint8_t *bitmap = NULL;
        uint8_t char_width = 0, char_height = 0;

        // 查找对应的位图
        for (int j = 0; j < glyph_count; ++j) {
            if (strcmp(utf8_char, glyphs[j].character) == 0) {
                bitmap = glyphs[j].data;
                char_width = glyphs[j].width;
                char_height = glyphs[j].height;                
                break;
            }
        }

        if (bitmap) {
            //ESP_LOGI("EPD", "x=%d,y=%d",placements[char_index].x, placements[char_index].y); 
            DrawBitmapToBuffer(placements[char_index].x, placements[char_index].y, bitmap, char_width, char_height, BLACK);
        }

        i += len;
        char_index++;  // 增加字符索引
    }

    EPD_Display(ImageBW);
    EPD_Update();
}

// 显示配网步骤到电子纸屏幕
void EPD_ShowNetworkConfigSteps(void)
{
    EPD_ShowChinese(144,10,(uint8_t*)"配网步骤",24,BLACK);
    EPD_ShowString(10,40,(uint8_t*)"1.",24,BLACK,WHITE);
    EPD_ShowChinese(34,40,(uint8_t*)"连接热点",24,BLACK);
    EPD_ShowString(130,40,(uint8_t*)":QUOTE",24,BLACK,WHITE);
    EPD_ShowString(10,70,(uint8_t*)"2.",24,BLACK,WHITE);
    EPD_ShowChinese(34,70,(uint8_t*)"浏览器访问",24,BLACK);
    EPD_ShowString(154,70,(uint8_t*)":192.168.4.1",24,BLACK,WHITE);
    EPD_ShowString(10,100,(uint8_t*)"3.",24,BLACK,WHITE);
    EPD_ShowChinese(34,100,(uint8_t*)"配置",24,BLACK);
    EPD_ShowString(82,100,(uint8_t*)"WiFi",24,BLACK,WHITE);
    EPD_ShowChinese(130,100,(uint8_t*)"名称和密码",24,BLACK);
    EPD_Update();                                 // 刷新屏幕显示内容
    EPD_Display(ImageBW);                         // 将画布内容发送到SRAM
    EPD_Update();                                 // 刷新SRAM内容显示到墨水屏
}

void app_main(void)
{
    print_chip_info();          // 打印芯片信息
    start_memory_monitor_task();// 启动内存监控任务
    init_nvs();                 // 初始化 NVS
    wifi_init_result_t result = wifi_init(); // 初始化WiFi

    EPD_Init();                                   // 墨水屏初始化
    Paint_NewImage(ImageBW,EPD_W,EPD_H,0,WHITE);  // 创建画布，画布数据存放于数组ImageBW
    Paint_Clear(WHITE);                           // 画布清屏

    if (result == WIFI_INIT_CONNECTED) {            //wiFi已连接才去服务器获取内容
        register_quote_display_callback(display_quote_on_epaper);   // 注册显示函数
        start_quote_fetch_task();                                   // 启动语录获取任务
    }else{
        EPD_ShowNetworkConfigSteps();               // 显示配网步骤
    }

    while(1){
        vTaskDelay(1000 / portTICK_PERIOD_MS);      // 非阻塞延时，用于任务切换
    }

}
