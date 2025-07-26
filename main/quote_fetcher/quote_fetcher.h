#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MAX_BITMAPS 64          // 一次语录不超过 64 个
#define BITMAP_SIZE 32          // 每个汉字占 16x16 位图，共 32 字节

#define MAX_UTF8_CHAR_LEN 4     // UTF8单个字符最多 4 字节
#define UTF8_CHAR_BUF_LEN (MAX_UTF8_CHAR_LEN + 1)  // +1 for '\0'

// 定义一个结构体来存储每个utf8的字符和位图
typedef struct {
    char character[UTF8_CHAR_BUF_LEN];        
    uint8_t data[BITMAP_SIZE];
} GlyphBitmap;

// 启动语录抓取任务
void start_quote_fetch_task(void);  

// 回调类型定义
typedef void (*quote_display_callback_t)(const char *quote, const GlyphBitmap *bitmaps, int count);

// 注册显示回调
void register_quote_display_callback(quote_display_callback_t callback);

#ifdef __cplusplus
}
#endif
