#include "quote_fetcher.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"
#include "esp_mac.h"
#include <esp_sleep.h>
#include "nvs_flash.h"
#include "driver/rtc_io.h"

#define TAG "QUOTE"

#define WAKE_PIN    34          // 34号引脚用于外部唤醒（连接按键）

// 全局变量保存回调函数
static quote_display_callback_t display_callback = NULL;


#define MAX_URL_LEN 256

#define REFRESH_INTERVAL_US 3600000000ULL    // 10分钟=600000000ULL，60分钟=3600000000ULL,单位微秒

#define NVS_NAMESPACE "epaper_quote"       // NVS命名空间（用于存储last_quote）
#define NVS_KEY_LAST_QUOTE "last_quote"    // NVS中存储last_quote的键
#define LAST_QUOTE_MAX_LEN 256             // 语录最大长度

esp_err_t generate_device_request_url(char *url_out, size_t max_len, request_reason_t reason)
{
    // 参数合法性检查
    if (url_out == NULL || max_len < 64) {  // 最小URL长度估算
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t mac[6];
    esp_err_t err = esp_read_mac(mac, ESP_MAC_WIFI_STA);  // 读取STA模式下的MAC地址

    if (err != ESP_OK) {
        memset(url_out, 0, max_len);    // 清空输出
        printf("获取MAC地址失败: %s\n", esp_err_to_name(err));
        return ESP_ERR_INVALID_ARG;
    }

    // 格式化MAC地址为不带冒号的大写字符串
    char mac_str[13];  // 12位十六进制 + 结束符
    snprintf(mac_str, sizeof(mac_str),
             "%02X%02X%02X%02X%02X%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // 3. 转换请求原因为字符串（便于服务器解析）
    const char *reason_str;
    switch (reason) {
        case REQUEST_REASON_BUTTON_TRIGGER: // 按键触发请求
            reason_str = "button_trigger";
            break;
        case REQUEST_REASON_TIMER:          // 定时请求
            reason_str = "timer";
            break;
        case REQUEST_REASON_MOTION_DETECT:  // 运动检测触发
            reason_str = "motion_detect";
            break;
        case REQUEST_REASON_ERROR:          // 错误状态上报
            reason_str = "error";
            break;
        default:                            // 防止未定义行为
            reason_str = "unknown";
            break;
    }

    // 拼接URL字符串
    int ret = snprintf(url_out, max_len,
                      "%s?mac=%s&reason=%s",
                      BASE_URL, mac_str, reason_str);

    // 检查URL是否被截断
    if (ret < 0 || (size_t)ret >= max_len) {
        memset(url_out, 0, max_len);
        return ESP_ERR_NO_MEM;  // 缓冲区不足
    }

    return ESP_OK;                      
}


// 注册显示回调函数
void register_quote_display_callback(quote_display_callback_t callback) {
    display_callback = callback;
}

// HTTP事件处理函数
static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    static int total_len = 0;
    char *buffer = (char *)evt->user_data;
    int buffer_size = 10240; // 10 KB

    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // 判断是否还有空间追加
                if (total_len + evt->data_len < buffer_size - 1) {
                    memcpy(buffer + total_len, evt->data, evt->data_len);
                    total_len += evt->data_len;
                    buffer[total_len] = '\0';  // 保证字符串结尾
                } else {
                    ESP_LOGW(TAG, "响应内容超出缓冲区，已截断");
                }
            }
            break;

        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(TAG, "HTTP 响应完整内容: %s", buffer);
            total_len = 0; // 重置
            break;

        default:
            break;
    }
    return ESP_OK;
}


static void fetch_quote_task(void *pvParameters) {
    while (1) {
        // 定义局部变量保存响应内容
        char quote_buffer[1024*10] = {0};
        // 获取带有 MAC 地址的 URL
        char full_url[MAX_URL_LEN];
        // 获取唤醒原因
        esp_sleep_wakeup_cause_t wake_cause = esp_sleep_get_wakeup_cause();

        // 映射为请求原因
        request_reason_t req_reason;
        switch (wake_cause) {
            case ESP_SLEEP_WAKEUP_EXT0:
            case ESP_SLEEP_WAKEUP_EXT1:
                req_reason = REQUEST_REASON_BUTTON_TRIGGER;  // 按键触发
                break;
            case ESP_SLEEP_WAKEUP_TIMER:
                req_reason = REQUEST_REASON_TIMER;           // 定时触发
                break;
            default:
                req_reason = REQUEST_REASON_UNKNOWN;         // 未知原因: 上电复位...
                break;
        }        
        generate_device_request_url(full_url, MAX_URL_LEN, req_reason);
        printf("请求 URL: %s\n", full_url);

        esp_http_client_config_t config = {
            .url = full_url,
            .event_handler = http_event_handler,
            .buffer_size = 1024*10,                 // 设置缓冲区大小,最大10KB
            .user_data = quote_buffer,              // 将缓冲区传递给事件处理函数
        };

        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_http_client_set_header(client, "Accept", "application/json");

        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
            int status = esp_http_client_get_status_code(client);
            ESP_LOGI(TAG, "HTTP 状态码: %d", status);
            ESP_LOGI(TAG, "响应数据长度: %d", strlen(quote_buffer));


            if (status == 200 && strlen(quote_buffer) > 0) {
                cJSON *root = cJSON_Parse(quote_buffer);
                if (root) {
                    cJSON *quote = cJSON_GetObjectItem(root, "quote");
                    cJSON *screen_model = cJSON_GetObjectItem(root, "screen_model");
                    cJSON *fontsize = cJSON_GetObjectItem(root, "fontsize");
                    cJSON *bitmaps = cJSON_GetObjectItem(root, "bitmaps");
                    cJSON *positions = cJSON_GetObjectItem(root, "positions");

                    GlyphBitmap glyphs[MAX_BITMAPS];   
                    int glyph_count = 0;
                    GlyphPlacement placements[MAX_BITMAPS];
                    int placement_count = 0;  

                    if (positions) {
                        cJSON *pos_item = NULL;
                        cJSON_ArrayForEach(pos_item, positions) {
                            if (placement_count >= MAX_BITMAPS) break;

                            cJSON *gIndexItem = cJSON_GetObjectItem(pos_item, "index");
                            cJSON *xItem      = cJSON_GetObjectItem(pos_item, "x");
                            cJSON *yItem      = cJSON_GetObjectItem(pos_item, "y");

                            if (!cJSON_IsNumber(gIndexItem) || !cJSON_IsNumber(xItem) || !cJSON_IsNumber(yItem)) {
                                continue;
                            }

                            placements[placement_count].glyph_index = (uint16_t)gIndexItem->valueint;
                            placements[placement_count].x = (int16_t)xItem->valueint;
                            placements[placement_count].y = (int16_t)yItem->valueint;
                            placement_count++;
                        }
                    }                    

                    if (quote && screen_model && fontsize && bitmaps && positions) {
                        int font_size = fontsize->valueint;                 // 获取字号
                        int bytes_per_char = (font_size * font_size) / 8;   // 计算每个字符的字节数                    
                        ESP_LOGI(TAG, "语录内容: %s, 屏幕型号：%s, 字号: %d", quote->valuestring, screen_model->valuestring, font_size);

                        cJSON *glyph_item = NULL;
                        cJSON_ArrayForEach(glyph_item, bitmaps) {   // 逐个处理json消息的bitmaps字段对象
                            if (glyph_count >= MAX_BITMAPS) break;

                            const char *key = glyph_item->string;   // 获取当前对象的键名
                            cJSON *array = glyph_item;              // 获取当前对象的值（数组）
                            if (!array || !cJSON_IsArray(array)) continue;  // 确保值是数组类型

                            // 复制键名到 GlyphBitmap 的 character 字段
                            strncpy(glyphs[glyph_count].character, key, sizeof(glyphs[glyph_count].character) - 1); // 复制键名到 GlyphBitmap 的 character 字段
                            glyphs[glyph_count].character[sizeof(glyphs[glyph_count].character) - 1] = '\0';    // 确保字符串以 null 结尾

                            // 分配动态内存
                            glyphs[glyph_count].data = malloc(bytes_per_char);
                            if (!glyphs[glyph_count].data) {
                                ESP_LOGE(TAG, "内存分配失败");
                                continue;
                            }

                            // 复制数组数据到 GlyphBitmap 的 data 字段
                            int i = 0;
                            cJSON *num = NULL;
                            cJSON_ArrayForEach(num, array) {
                                if (i >= bytes_per_char) break;
                                glyphs[glyph_count].data[i++] = (uint8_t)cJSON_GetNumberValue(num);
                            }
                            glyphs[glyph_count].width = font_size;
                            glyphs[glyph_count].height = font_size;
                            glyph_count++;
                        }

                        if (display_callback) {
                            display_callback(screen_model->valuestring, quote->valuestring, glyphs, glyph_count, placements);
                        }

                        // 释放内存
                        for (int i = 0; i < glyph_count; ++i) {
                            if (glyphs[i].data) {
                                free(glyphs[i].data);
                            }
                        }

                    } else {
                        ESP_LOGE(TAG, "字段 quote 或 bitmaps 无效");
                    }
                    cJSON_Delete(root);
                } else {
                    ESP_LOGE(TAG, "JSON 解析失败");
                }
            } else {
                ESP_LOGE(TAG, "状态码错误或无响应数据");
            }
        } else {
            ESP_LOGE(TAG, "请求失败: %s", esp_err_to_name(err));
        }

        esp_http_client_cleanup(client);

        // 监控当前任务或指定任务的剩余栈空间
        // UBaseType_t high_water_mark = uxTaskGetStackHighWaterMark(NULL);
        // ESP_LOGI("QUOTE", "Minimum free stack space: %d bytes", high_water_mark * 4);

        //vTaskDelay(pdMS_TO_TICKS(10 * 60 * 1000));  // 10分钟后再请求

        // 进入深度睡眠
        vTaskDelay(pdMS_TO_TICKS(3 * 1000));   // 确保墨水屏刷新完毕
        ESP_LOGI(TAG, "开始深度睡眠");

        rtc_gpio_deinit(WAKE_PIN);              // 切换为RTC模式
        gpio_reset_pin(WAKE_PIN);               // 复位GPIO配置
        rtc_gpio_init(WAKE_PIN);                // 初始化RTC引脚
        rtc_gpio_set_direction(WAKE_PIN, RTC_GPIO_MODE_INPUT_ONLY); // 输入模式
        esp_sleep_enable_ext0_wakeup(WAKE_PIN , 0);                 // 34号引脚低电平唤醒
        esp_sleep_enable_timer_wakeup(REFRESH_INTERVAL_US);         // 定时唤醒
        esp_deep_sleep_start();

    }
}



void start_quote_fetch_task(void) {
    xTaskCreate(fetch_quote_task, "quote_task", 1024*15, NULL, 5, NULL);
}


// 从NVS读取上一次的语录
esp_err_t nvs_read_last_quote(char *last_quote, size_t max_len) {
    esp_err_t err;
    nvs_handle_t nvs_handle;

    // 打开NVS命名空间
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Open namespace failed: %s", esp_err_to_name(err));
        return err;
    }

    // 读取last_quote（默认初始化为空字符串）
    size_t len = max_len;
    err = nvs_get_str(nvs_handle, NVS_KEY_LAST_QUOTE, last_quote, &len);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI("NVS", "No last quote in NVS, init as empty");
        last_quote[0] = '\0';  // 首次运行时，设为空字符串
        err = ESP_OK;
    } else if (err != ESP_OK) {
        ESP_LOGE("NVS", "Read last quote failed: %s", esp_err_to_name(err));
    }

    nvs_close(nvs_handle);
    return err;
}


// 向NVS写入最新的语录
esp_err_t nvs_write_last_quote(const char *new_quote) {
    esp_err_t err;
    nvs_handle_t nvs_handle;

    // 打开NVS命名空间
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Open namespace failed: %s", esp_err_to_name(err));
        return err;
    }

    // 写入新语录（会覆盖旧值）
    err = nvs_set_str(nvs_handle, NVS_KEY_LAST_QUOTE, new_quote);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Write last quote failed: %s", esp_err_to_name(err));
    } else {
        // 提交写入（NVS需要手动提交才会生效）
        err = nvs_commit(nvs_handle);
        if (err != ESP_OK) {
            ESP_LOGE("NVS", "Commit last quote failed: %s", esp_err_to_name(err));
        }
    }

    nvs_close(nvs_handle);
    return err;
}





