#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// 启动语录抓取任务
void start_quote_fetch_task(void);  

// 回调类型定义
typedef void (*quote_display_callback_t)(const char *quote);

// 注册显示回调
void register_quote_display_callback(quote_display_callback_t callback);

#ifdef __cplusplus
}
#endif
