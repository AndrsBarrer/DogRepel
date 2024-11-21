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

/**
 * @brief Starts the HTTP server and registers URI handlers.
 * 
 * This function initializes and starts the HTTP server with a custom configuration, registers URI handlers
 * for specific endpoints, and returns the server handle if the server starts successfully.
 * 
 * - Configures the server with default settings and custom stack size.
 * - Starts the HTTP server using the configuration.
 * - Registers URI handlers to handle requests for the root URI and process information.
 * 
 * @return httpd_handle_t The server handle if the server starts successfully, NULL otherwise.
 */
httpd_handle_t start_webserver(void);

/**
 * @brief Initializes the ESP32 Wi-Fi SoftAP (Access Point) mode.
 * 
 * This function initializes the Wi-Fi interface, sets up the SoftAP mode, and starts the Wi-Fi network with
 * a configured SSID, password, and channel. It also registers an event handler for Wi-Fi events and logs the
 * AP's IP address.
 * 
 * - Initializes the network interface and default Wi-Fi settings.
 * - Creates a default Wi-Fi AP interface.
 * - Configures Wi-Fi settings for the AP (SSID, password, channel, etc.).
 * - Starts the Wi-Fi in AP mode and logs the AP's IP address.
 * 
 * @note If no password is set, the AP will use an open authentication mode.
 */






/**
 * @brief Initializes the ESP32 device as a Wi-Fi Access Point (AP).
 * 
 * This function configures the device to operate in Access Point mode, with 
 * specific settings like SSID, password, and maximum connections. It also 
 * registers event handlers for Wi-Fi events and starts the Wi-Fi stack.
 * 
 * The function sets the SSID and password based on the defined macros 
 * `ESP_WIFI_AP_SSID` and `ESP_WIFI_AP_PASSWD`. If the password is empty, 
 * the AP will be open without encryption.
 * 
 * @note The AP's IP address will be printed to the log once the device 
 * is initialized and running.
 * 
 * @return void
 */
void wifi_init_softap(void);
#ifdef STATION
/**
 * @brief Initializes the ESP32 device as a Wi-Fi Station (STA).
 * 
 * This function initializes the NVS (Non-Volatile Storage) and retrieves the stored 
 * Wi-Fi SSID and password for the connection. It sets up the device to connect to 
 * a Wi-Fi network in Station mode. If the SSID or password is not found in the NVS, 
 * default behavior is applied (e.g., resetting the flash memory).
 * 
 * @note The function assumes that the Wi-Fi credentials are stored in NVS. If they 
 * are not found, appropriate fallback handling should be implemented (such as using 
 * default values or initiating a reset).
 * 
 * @return void
 */
void wifi_init_sta(void);

/**
 * @brief Event handler for Wi-Fi and IP events in Station mode.
 * 
 * This function handles various Wi-Fi and IP events in Station mode. It processes 
 * events such as Wi-Fi connection start, disconnection, and IP acquisition. 
 * Based on the event type, it takes appropriate actions such as attempting to 
 * reconnect to Wi-Fi, starting a TCP client task upon successful IP acquisition, 
 * and logging the event information.
 * 
 * - On `WIFI_EVENT_STA_START`: Initiates Wi-Fi connection.
 * - On `WIFI_EVENT_STA_DISCONNECTED`: Attempts to reconnect to Wi-Fi when disconnected.
 * - On `IP_EVENT_STA_GOT_IP`: Starts the TCP client task after acquiring an IP address.
 * 
 * @param arg The argument passed to the event handler (not used here).
 * @param event_base The event base identifying the event type.
 * @param event_id The specific event ID.
 * @param event_data The event data, which contains additional event-specific information.
 * 
 * @return void
 */
void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

#else
/**
 * @brief Initializes the ESP32 device as a Wi-Fi Station (STA).
 * 
 * This function initializes the ESP32 device for Station mode and configures it 
 * to connect to a Wi-Fi network using the provided SSID and password. It also 
 * sets up event handlers for Wi-Fi and IP events, enabling the device to handle 
 * Wi-Fi connection events and obtain an IP address.
 * 
 * It uses WPA3 security features and battery-saving scan methods. If the Wi-Fi 
 * network is not found, the device will attempt to reconnect a set number of times.
 * 
 * @note Ensure that the `WIFI_DOG_REPEL_SSID` and `WIFI_DOG_REPEL_PASS` macros 
 * are defined with valid credentials before calling this function.
 * 
 * @return void
 */
void wifi_init_sta(void);

/**
 * @brief Event handler for Wi-Fi and IP events in Station mode.
 * 
 * This function handles various Wi-Fi and IP events in Station mode. It processes 
 * events such as Wi-Fi connection start, disconnection, and IP acquisition. 
 * Based on the event type, it takes appropriate actions such as attempting to 
 * reconnect to Wi-Fi, starting a TCP client task upon successful IP acquisition, 
 * and logging the event information.
 * 
 * - On `WIFI_EVENT_STA_START`: Initiates Wi-Fi connection.
 * - On `WIFI_EVENT_STA_DISCONNECTED`: Attempts to reconnect to Wi-Fi when disconnected.
 * - On `IP_EVENT_STA_GOT_IP`: Starts the TCP client task after acquiring an IP address.
 * 
 * @param arg The argument passed to the event handler (not used here).
 * @param event_base The event base identifying the event type.
 * @param event_id The specific event ID.
 * @param event_data The event data, which contains additional event-specific information.
 * 
 * @return void
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
#endif