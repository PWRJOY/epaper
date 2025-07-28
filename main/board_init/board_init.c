#include "board_init.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_system.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void print_chip_info(void) {
    ESP_LOGI("BUILD", "%s", BUILD_ID);          

    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);

    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
}




void init_nvs(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW("nvs", "NVS error, erasing flash and retrying...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}




void memory_monitor_task(void *pvParameters)
{
    (void) pvParameters; // 明确标记参数未使用

    while (1) {
        // 获取当前空闲堆内存和历史最小堆内存
        size_t free_heap = esp_get_free_heap_size();
        size_t min_free_heap = esp_get_minimum_free_heap_size();
        // 监控当前任务或指定任务的剩余栈空间
        //UBaseType_t high_water_mark = uxTaskGetStackHighWaterMark(NULL);

        ESP_LOGI("MemoryMonitor", "Free heap memory: %d bytes, Minimum free heap memory: %d bytes", free_heap, min_free_heap);
        //ESP_LOGI("MemoryMonitor", "Minimum free stack space: %d bytes", high_water_mark * 4); //2048-640= 1408

        // 每10分钟获取一次
        vTaskDelay(10*60*1000 / portTICK_PERIOD_MS);
    }
}
void start_memory_monitor_task(void) {
    xTaskCreate(memory_monitor_task, "MemoryMonitor", 2048, NULL, 5, NULL);
}