#pragma once
#include <stdbool.h>
#include "esp_err.h"
#include "esp_wifi_types.h"

typedef enum {
    WIFI_EXT_OPEN = 0,             // Open network
    WIFI_EXT_WPA2 = 1,             // WPA2-Personal
    WIFI_EXT_WPA3 = 2,             // WPA3-Personal
    WIFI_EXT_OWE = 3,              // Enhanced Open (OWE)
    WIFI_EXT_ENTERPRISE_WPA2 = 4,  // WPA2-Enterprise (login+password)
    WIFI_EXT_ENTERPRISE_WPA3 = 5   // WPA3-Enterprise (login+password)
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