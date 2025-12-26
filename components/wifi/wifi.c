#include "wifi.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_eap_client.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "wifi";

static bool s_initialized = false;
static bool s_connected = false;
static esp_netif_t *s_netif_sta = NULL;
static wifi_ap_record_t *s_scan_results = NULL;
static uint16_t s_scan_count = 0;

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "STA started");
                break;
            case WIFI_EVENT_STA_DISCONNECTED:
                s_connected = false;
                ESP_LOGW(TAG, "STA disconnected");
                break;
            case WIFI_EVENT_SCAN_DONE:
                esp_wifi_scan_get_ap_num(&s_scan_count);
                free(s_scan_results);
                s_scan_results = NULL;
                if (s_scan_count > 0) {
                    s_scan_results = calloc(s_scan_count, sizeof(wifi_ap_record_t));
                    esp_wifi_scan_get_ap_records(&s_scan_count, s_scan_results);
                }
                ESP_LOGI(TAG, "Scan done (%d APs)", s_scan_count);
                break;
            default: break;
        }
    }
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        s_connected = true;
        ESP_LOGI(TAG, "Got IP");
    }
}

esp_err_t wifi_init(void)
{
    if (s_initialized) return ESP_OK;

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    s_netif_sta = esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(esp_netif_set_hostname(s_netif_sta, CONFIG_WIFI_HOSTNAME));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    s_initialized = true;
    ESP_LOGI(TAG, "WiFi initialized");
    return ESP_OK;
}

esp_err_t wifi_scan_start(void)
{
    if (!s_initialized) return ESP_ERR_INVALID_STATE;
    wifi_scan_config_t scan_cfg = {0};
    scan_cfg.show_hidden = true;
    return esp_wifi_scan_start(&scan_cfg, false);
}

uint16_t wifi_scan_get_count(void) { return s_scan_count; }
wifi_ap_record_t *wifi_scan_get_results(void) { return s_scan_results; }

esp_err_t wifi_connect(const char *ssid,
                       const char *password,
                       wifi_auth_mode_ext_t auth)
{
    if (!s_initialized || !ssid) return ESP_ERR_INVALID_ARG;

    wifi_config_t cfg = {0};
    strncpy((char *)cfg.sta.ssid, ssid, sizeof(cfg.sta.ssid) - 1);
    cfg.sta.pmf_cfg.capable = true;
    cfg.sta.pmf_cfg.required = false;

    switch(auth) {
        case WIFI_EXT_OPEN:
            cfg.sta.threshold.authmode = WIFI_AUTH_OPEN;
            break;
        case WIFI_EXT_WPA2:
            cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
            strncpy((char *)cfg.sta.password, password, sizeof(cfg.sta.password)-1);
            break;
        case WIFI_EXT_WPA3:
            cfg.sta.threshold.authmode = WIFI_AUTH_WPA3_PSK;
            strncpy((char *)cfg.sta.password, password, sizeof(cfg.sta.password)-1);
            break;
        case WIFI_EXT_OWE:
            cfg.sta.threshold.authmode = WIFI_AUTH_OPEN;
            cfg.sta.owe_enabled = true;
            break;
        default:
            return ESP_ERR_INVALID_ARG;
    }

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));
    return esp_wifi_connect();
}

esp_err_t wifi_connect_enterprise(const char *ssid,
                                  const char *identity,
                                  const char *username,
                                  const char *password,
                                  wifi_auth_mode_ext_t auth)
{
    if (!s_initialized || !ssid || !identity || !password) return ESP_ERR_INVALID_ARG;

    wifi_config_t cfg = {0};
    strncpy((char *)cfg.sta.ssid, ssid, sizeof(cfg.sta.ssid) - 1);
    cfg.sta.pmf_cfg.capable = true;
    cfg.sta.pmf_cfg.required = (auth == WIFI_EXT_ENTERPRISE_WPA3);

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));

    ESP_ERROR_CHECK(esp_eap_client_set_identity((uint8_t *)identity, strlen(identity)));

    if (username) {
        ESP_ERROR_CHECK(esp_eap_client_set_username((uint8_t *)username, strlen(username)));
    }

    ESP_ERROR_CHECK(esp_eap_client_set_password((uint8_t *)password, strlen(password)));

    ESP_ERROR_CHECK(esp_wifi_sta_enterprise_enable());
    return esp_wifi_connect();
}

esp_err_t wifi_disconnect(void)
{
    if (!s_initialized) return ESP_ERR_INVALID_STATE;
    return esp_wifi_disconnect();
}

bool wifi_is_connected(void) { return s_connected; }