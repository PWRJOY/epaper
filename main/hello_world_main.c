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


void app_main(void)
{
    print_chip_info();          // 打印芯片信息
    start_memory_monitor_task();// 启动内存监控任务
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
