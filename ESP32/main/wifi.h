#include "esp_wifi.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_pm.h"
#include "esp_sleep.h"

#include <stdbool.h>

#include "config.h"

extern bool storedSSID;
extern bool storedPASS;


/* AP Configuration */
#define ESP_WIFI_AP_SSID "MyWifi"
#define ESP_WIFI_AP_PASSWD "12345678"
#define ESP_WIFI_CHANNEL 1
#define MAX_STA_CONN 4

/* STA Configuration */
#define WIFI_DOG_REPEL_SSID "Dog_Repel"
#define WIFI_DOG_REPEL_PASS "123456789"
#define ESP_MAXIMUM_RETRY 5
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK

extern char got_collar_mac[18];
#define IP_ADDR_STR_LEN 20
extern char server_ip[IP_ADDR_STR_LEN];

extern char station_mac_addr_str[20];
extern int8_t currentRSSI;

#define PORT 2222

httpd_handle_t start_webserver(void);
void wifi_init_softap(void);
void wifi_init_sta(void);
void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);