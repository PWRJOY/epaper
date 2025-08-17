#include "wifi.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "freertos/event_groups.h"
#include "config.h"
#include "esp_http_server.h"
#include "wifi_html.h"
#include "driver/gpio.h"

#define WIFI_CONNECTED_BIT BIT0
static EventGroupHandle_t wifi_event_group;
static const char *TAG = "wifi";

static char saved_ssid[32] = {0};
static char saved_pass[64] = {0};

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "WiFi disconnected");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// 保存 WiFi 信息到 NVS
static void save_wifi_config(const char *ssid, const char *pass) {
    nvs_handle_t nvs;
    ESP_ERROR_CHECK(nvs_open("wifi", NVS_READWRITE, &nvs));
    nvs_set_str(nvs, "ssid", ssid);
    nvs_set_str(nvs, "pass", pass);
    nvs_commit(nvs);
    nvs_close(nvs);
}

// 读取 WiFi 信息
static bool load_wifi_config(char *ssid, char *pass) {
    nvs_handle_t nvs;
    size_t ssid_len = 32, pass_len = 64;
    if (nvs_open("wifi", NVS_READONLY, &nvs) != ESP_OK) return false;
    if (nvs_get_str(nvs, "ssid", ssid, &ssid_len) != ESP_OK ||
        nvs_get_str(nvs, "pass", pass, &pass_len) != ESP_OK) {
        nvs_close(nvs);
        return false;
    }
    nvs_close(nvs);
    return true;
}

// 启动 STA 模式
void wifi_init_sta(const char *ssid, const char *pass) {
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {0};
    strcpy((char *)wifi_config.sta.ssid, ssid);
    strcpy((char *)wifi_config.sta.password, pass);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}

// 添加URL解码辅助函数
static char hex_to_char(const char *hex) {
    char c = 0;
    if (*hex >= '0' && *hex <= '9') {
        c += (*hex - '0') << 4;
    } else if (*hex >= 'A' && *hex <= 'F') {
        c += (*hex - 'A' + 10) << 4;
    } else if (*hex >= 'a' && *hex <= 'f') {
        c += (*hex - 'a' + 10) << 4;
    } else {
        return 0; // 无效的十六进制字符
    }
    
    hex++;
    if (*hex >= '0' && *hex <= '9') {
        c += (*hex - '0');
    } else if (*hex >= 'A' && *hex <= 'F') {
        c += (*hex - 'A' + 10);
    } else if (*hex >= 'a' && *hex <= 'f') {
        c += (*hex - 'a' + 10);
    } else {
        return 0; // 无效的十六进制字符
    }
    
    return c;
}

// URL解码函数
static void url_decode(char *dst, const char *src) {
    while (*src) {
        if (*src == '%' && *(src+1) && *(src+2)) {
            *dst++ = hex_to_char(src + 1);
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' '; // 将+转换为空格
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

static esp_err_t wifi_post_handler(httpd_req_t *req) {
    char buf[100];
    int ret = httpd_req_recv(req, buf, sizeof(buf)-1);
    if (ret <= 0) {
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    char ssid_encoded[32], pass_encoded[64];
    char ssid[32], pass[64];
    
    // 先提取编码后的参数
    if (sscanf(buf, "ssid=%31[^&]&pass=%63s", ssid_encoded, pass_encoded) != 2) {
        httpd_resp_sendstr(req, "参数格式错误");
        return ESP_FAIL;
    }
    
    // 对提取的参数进行URL解码
    url_decode(ssid, ssid_encoded);
    url_decode(pass, pass_encoded);
    
    // 调试输出解码前后的值，确认解码是否正确
    ESP_LOGI(TAG, "编码后的SSID: %s", ssid_encoded);
    ESP_LOGI(TAG, "解码后的SSID: %s", ssid);
    ESP_LOGI(TAG, "编码后的密码: %s", pass_encoded);
    ESP_LOGI(TAG, "解码后的密码: %s", pass);

    save_wifi_config(ssid, pass);
    httpd_resp_sendstr(req, "配置已保存，设备即将重启");
    esp_restart();
    return ESP_OK;
}

static esp_err_t wifi_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, wifi_config_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t favicon_handler(httpd_req_t *req) {
    const char empty[] = "";
    httpd_resp_set_type(req, "image/x-icon");
    httpd_resp_send(req, empty, sizeof(empty));
    return ESP_OK;
}


static void start_http_server(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG(); 
    httpd_start(&server, &config);

    httpd_uri_t favicon_uri = {
        .uri = "/favicon.ico",
        .method = HTTP_GET,
        .handler = favicon_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &favicon_uri);

    // GET 页面
    httpd_uri_t wifi_get_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = wifi_get_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &wifi_get_uri);

    // POST 数据
    httpd_uri_t wifi_post_uri = {
        .uri = "/wifi",
        .method = HTTP_POST,
        .handler = wifi_post_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &wifi_post_uri);
}


// 启动 AP 模式
static void start_ap_mode(void) {
    // ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t ap_config = {
        .ap = {
            .ssid = "ESP32_Config",
            .ssid_len = strlen("QUOTE"),
            .password = "88888888",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    start_http_server();
}

static void trigger_gpio_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << CONFIG_TRIGGER_AP_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,  // 启用上拉，确保未连接时为高电平
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE     // 不需要中断
    };
    gpio_config(&io_conf);
}

wifi_init_result_t wifi_init(void)
{
    
    // 检查是否需要强制进入AP模式
    trigger_gpio_init();
    bool force_ap_mode = gpio_get_level(CONFIG_TRIGGER_AP_GPIO) == 0;

    if (!force_ap_mode && load_wifi_config(saved_ssid, saved_pass)) {
        ESP_LOGI(TAG, "加载到的WiFi配置 - SSID: '%s'", saved_ssid);
        ESP_LOGI(TAG, "加载到的WiFi配置 - 密码: '%s'", saved_pass);
        wifi_init_sta(saved_ssid, saved_pass);
        EventBits_t bits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(5000));
        if (bits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi 已连接");
            return WIFI_INIT_CONNECTED;  // 返回连接成功状态
        } else {
            ESP_LOGI(TAG, "WiFi 连接失败，进入 AP 配网模式");
            start_ap_mode();
            return WIFI_INIT_AP_MODE;       // 返回进入AP模式状态
        }
    } else {
        if (force_ap_mode) {
            ESP_LOGI(TAG, "GPIO%d为低电平，强制进入 AP 配网模式", CONFIG_TRIGGER_AP_GPIO);
        } else {
            ESP_LOGI(TAG, "没有保存的 WiFi，进入 AP 配网模式");
        }        
        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        start_ap_mode();
        return WIFI_INIT_AP_MODE;       // 返回进入AP模式状态
    }
}
