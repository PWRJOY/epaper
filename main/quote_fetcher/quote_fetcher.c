#include "quote_fetcher.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "config.h"


#define TAG "QUOTE"



// 全局变量保存回调函数
static quote_display_callback_t display_callback = NULL;

// 注册显示回调函数
void register_quote_display_callback(quote_display_callback_t callback) {
    display_callback = callback;
}


// 全局缓冲区保存响应内容
static char quote_buffer[512] = {0};  

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // 拷贝响应数据到 quote_buffer（确保不溢出）
                int copy_len = evt->data_len < sizeof(quote_buffer) - 1 ? evt->data_len : sizeof(quote_buffer) - 1;
                memcpy(quote_buffer, evt->data, copy_len);
                quote_buffer[copy_len] = '\0';
                ESP_LOGI(TAG, "HTTP 响应内容: [%s]", quote_buffer);
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}

static void fetch_quote_task(void *pvParameters) {
    while (1) {
        // 清空缓冲区
        memset(quote_buffer, 0, sizeof(quote_buffer));

        esp_http_client_config_t config = {
            .url = API_URL,
            .event_handler = http_event_handler,
            .buffer_size = 512,
        };

        esp_http_client_handle_t client = esp_http_client_init(&config);
        esp_http_client_set_header(client, "Accept", "application/json");

        esp_err_t err = esp_http_client_perform(client);
        if (err == ESP_OK) {
            int status = esp_http_client_get_status_code(client);
            ESP_LOGI(TAG, "HTTP 状态码: %d", status);

            if (status == 200 && strlen(quote_buffer) > 0) {
                // 解析 JSON
                cJSON *root = cJSON_Parse(quote_buffer);
                if (root) {
                    cJSON *quote = cJSON_GetObjectItem(root, "quote");
                    if (cJSON_IsString(quote)) {
                        ESP_LOGI(TAG, "语录内容: %s", quote->valuestring);
                        // 调用回调函数显示语录
                        if (display_callback) {
                            display_callback(quote->valuestring);
                        }
                    } else {
                        ESP_LOGE(TAG, "字段 quote 无效");
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
        vTaskDelay(pdMS_TO_TICKS(10 * 60 * 1000));  // 10 分钟后再请求
    }
}




void start_quote_fetch_task(void) {
    xTaskCreate(fetch_quote_task, "quote_task", 8192, NULL, 5, NULL);
}
