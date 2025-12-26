#pragma once
#include <stdbool.h>
#include "esp_err.h"
#include "esp_wifi_types.h"

typedef enum {
    WIFI_EXT_OPEN,
    WIFI_EXT_WPA2,
    WIFI_EXT_WPA3,
    WIFI_EXT_OWE,
    WIFI_EXT_ENTERPRISE_WPA2,
    WIFI_EXT_ENTERPRISE_WPA3
} wifi_auth_mode_ext_t;

esp_err_t wifi_init(void);
esp_err_t wifi_scan_start(void);
uint16_t wifi_scan_get_count(void);
wifi_ap_record_t *wifi_scan_get_results(void);

esp_err_t wifi_connect(const char *ssid,
                       const char *password,
                       wifi_auth_mode_ext_t auth);

esp_err_t wifi_connect_enterprise(const char *ssid,
                                  const char *identity,
                                  const char *username,
                                  const char *password,
                                  wifi_auth_mode_ext_t auth);

esp_err_t wifi_disconnect(void);
bool wifi_is_connected(void);