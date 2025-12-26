#include <esp_log.h>
#include <stdio.h>
#include <wifi.h>
#include "freertos/FreeRTOS.h"
#include <freertos/task.h>

static const char* TAG = "main";

void app_main(void)
{
    // TODO implement command system

    wifi_init();
    wifi_scan_start();

    vTaskDelay(pdMS_TO_TICKS(5000));

    uint16_t ap_count = wifi_scan_get_count();
    ESP_LOGI(TAG, "Finded :kekw: %d", ap_count);

    wifi_ap_record_t *aps = wifi_scan_get_results();
    for (int i = 0; i < ap_count; i++) {
        ESP_LOGI(TAG, "SSID: %s  RSSI: %d AUTH: %d", aps[i].ssid, aps[i].rssi, aps[i].authmode);
    }
}
