// #define COLLAR_CODE
#define STATION_CODE

#include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/netdb.h"

#include "math.h"
#include "esp_mac.h"

#define LED GPIO_NUM_2

#define MAXIMUM_PWM 4000

// Variables to keep track of when in AP mode, they are not used in Station mode
static wifi_mode_t currentWifiMode = WIFI_MODE_AP;

/* --- Some configurations --- */
#define SSID_MAX_LEN (32 + 1) // max length of a SSID
#define MD5_LEN (32 + 1)      // length of md5 hash

#define ASSOCIATION_REQUEST 0x0;
#define ASSOCIATION_RESPONSE 0x1;
#define REASSOCIATION_REQUEST 0x2;
#define REASSOCIATION_RESPONSE 0x3;
#define PROBE_REQUEST 0x4;
#define PROBE_RESPONSE 0x5;
#define BEACON 0x8;
#define ATIM 0x9;
#define DISASSOCIATION 0xA;
#define AUTHENTICATION 0xB;
#define DEAUTHENTICATION 0xC;
#define ACTION 0xD;
#define ACTION_NO_ACK 0xE;

/* TAG of ESP32 for I/O operation */
static const char *TAG = "Sniff sniff";

static bool RUNNING = true;

#define CONFIG_CHANNEL 6
#define MAC_STR_LEN 18

#define MNGMT_PROBE_MASK 0x00F0
#define MNGMT_PROBE_PACKET 0x0040
#define MAX_DEVICES 10
#define COOLDOWN_PERIOD 5000 // Cooldown period in milliseconds

static uint8_t station_mac_addr[6] = {0};

// Function to resolve server IP using UDP broadcast
#define UDP_PORT 12345
#define MAX_RECEIVE_BUFFER_SIZE 128

#define BROADCAST_INTERVAL_MS 5000 // 5 seconds between retries
#define RECEIVE_TIMEOUT_MS 2000    // 2 seconds timeout for receiving

typedef struct
{
    int16_t fctl;                                 // frame control
    int16_t duration;                             // duration id, helps in managing the time allocation for transmission
    uint8_t DA[6];                                // destination address (MAC address of the device that is intended to receive the frame)
    uint8_t SA[6];                                // sender address (MAC address of the device that is sending the address)
    uint8_t BSS_ID[6];                            // MAC address of the AP in a WiFi network
    int16_t seq_ctl;                              // sequence control, may help in the acknowledgment process
    unsigned char payload[];                      // data payload of the WiFi frame
} __attribute__((packed)) wifi_management_header; // This attribute tells the compiler to pack the structure members together without padding

typedef struct
{
    uint8_t mac[6];              // Store the MAC of the device
    uint32_t last_time_received; // Store the last time the device mac was read
} packet_info_timeout;

// static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void sniffer_task(void *pvParameter);
static void wifi_sniffer_init(void);
static void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);
// static void tcp_client_task(void *pvParameters);
char **tokenizeString(char *str, const char *delim);
void wifi_init_sta();

void printMACS(packet_info_timeout mac_list[MAX_DEVICES], int size);
void printMAC(uint8_t mac[6]);
void formatMAC2STR(uint8_t mac[6], char *returnMACstr);

#ifdef COLLAR_CODE
#include "esp_pm.h"
#include "esp_sleep.h"

/* STA Configuration */
#define ESP_MAXIMUM_RETRY 5
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK

// Function prototypes
esp_err_t initIO();
void ADC1_Ch3_Ini(void);
float ADC1_Ch3_Read(void);
float ADC1_Ch3_Read_mV(void);
#endif