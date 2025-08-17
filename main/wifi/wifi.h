#pragma once

// 定义用于配网触发的GPIO引脚，可根据实际硬件修改
#define CONFIG_TRIGGER_AP_GPIO    33

// 定义WiFi初始化结果的枚举类型
typedef enum {
    WIFI_INIT_CONNECTED,    // WiFi已成功连接
    WIFI_INIT_AP_MODE       // 进入AP配网模式
} wifi_init_result_t;

wifi_init_result_t wifi_init(void);