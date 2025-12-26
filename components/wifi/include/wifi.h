#pragma once

#include "esp_err.h"
#include "esp_wifi.h"
#include <stdbool.h>
#include <stdint.h>

esp_err_t wifi_init(void);

esp_err_t wifi_scan_start(void);
uint16_t wifi_scan_get_count(void);
wifi_ap_record_t *wifi_scan_get_results(void);

esp_err_t wifi_connect(const char *ssid, const char *password);
esp_err_t wifi_disconnect(void);

bool wifi_is_connected(void);
