#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "esp_err.h"

#define MAX_BITMAPS 64          // 一次语录不超过 64 个

#define FONT_16_SIZE 32         // (16 * 16 / 8)
#define FONT_24_SIZE 72         // (24 * 24 / 8)
#define FONT_32_SIZE 128        // (32 * 32 / 8)
#define FONT_48_SIZE 288        // (48 * 48 / 8)
#define FONT_64_SIZE 512        // (64 * 64 / 8)
#define FONT_72_SIZE 1024       // (72 * 72 / 8)

#define MAX_UTF8_CHAR_LEN 4     // UTF8单个字符最多 4 字节
#define UTF8_CHAR_BUF_LEN (MAX_UTF8_CHAR_LEN + 1)  // +1 for '\0'

// 定义一个结构体来存储每个utf8的字符和位图
typedef struct {
    char character[UTF8_CHAR_BUF_LEN];        
    uint8_t *data;              // 位图数据指针（不再固定大小）
    uint8_t width;              // 宽度（像素）
    uint8_t height;             // 高度（像素）
} GlyphBitmap;

// 定义一个结构体来存储每个字符在画布上的位置
typedef struct {
    uint16_t glyph_index;       // 指向字符内容的下标，因为字模是去重过的
    int16_t x;
    int16_t y;
} GlyphPlacement;

// 启动语录抓取任务
void start_quote_fetch_task(void);  

// 回调类型定义
typedef void (*quote_display_callback_t)(const char *quote, const GlyphBitmap *bitmaps, int count, const GlyphPlacement *placements);

// 注册显示回调
void register_quote_display_callback(quote_display_callback_t callback);

esp_err_t nvs_read_last_quote(char *last_quote, size_t max_len);
esp_err_t nvs_write_last_quote(const char *new_quote);


#ifdef __cplusplus
}
#endif
