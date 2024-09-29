#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include "driver/gpio.h"
#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_system.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"

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

// This is going to be used as a timer cooldown for duplicate MACs
static uint32_t millis = 0;

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

#define WIFI_SSID "Ext_2.4"
#define WIFI_PASS "Moorparkcalifornia"
#define CONFIG_CHANNEL 6
#define MAC_STR_LEN 18

#define MNGMT_PROBE_MASK 0x00F0
#define MNGMT_PROBE_PACKET 0x0040
#define MAX_DEVICES 10
#define COOLDOWN_PERIOD 10000 // Cooldown period in milliseconds

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

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void sniffer_task(void *pvParameter);
static void wifi_sniffer_init(void);
static void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);
void wifi_init_sta();
void delayMs(uint16_t ms);
void printMACS(packet_info_timeout mac_list[MAX_DEVICES], int size);
void printMAC(uint8_t mac[6]);

void formatMAC2STR(uint8_t mac[6], char *returnMACstr);

void app_main(void)
{
    ESP_LOGI(TAG, "[+] Startup...");

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_sta();

    ESP_LOGI(TAG, "[!] Starting sniffing task...");
    xTaskCreate(&sniffer_task, "sniffer_task", 10000, NULL, 1, NULL);

    while (RUNNING)
    { // every 0.5s check if fatal error occurred
        delayMs(500);
    }
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Station started");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "Disconnected from Wi-Fi, trying to reconnect...");
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
    }
}

/* Initialize wifi station */
void wifi_init_sta(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

    wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_sta_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "wifi_init_sta finished.");
}

static void sniffer_task(void *pvParameter)
{
    ESP_LOGI(TAG, "[SNIFFER] Sniffer task created");
    wifi_sniffer_init();

    while (1)
    {
        millis++;
        delayMs(1);
    }
}

static void wifi_sniffer_init()
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    const wifi_country_t wifi_country = {
        .cc = "MX",
        .schan = 1,
        .nchan = 11,
        .policy = WIFI_COUNTRY_POLICY_AUTO};

    ESP_ERROR_CHECK(esp_wifi_set_country(&wifi_country));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
    // ESP_ERROR_CHECK(esp_wifi_set_channel(CONFIG_CHANNEL, WIFI_SECOND_CHAN_NONE));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&(wifi_promiscuous_filter_t){
        .filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT, // we only care about management type packets
    }));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler));
}

// Every time a packet is received, this function gets called
static void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type)
{
    // Store the buffer as a promiscuous packet so the payload can be extracted
    const wifi_promiscuous_pkt_t *packet = (wifi_promiscuous_pkt_t *)buff;

    // Convert it to something that we can use to extract ssid
    const wifi_management_header *header = (wifi_management_header *)packet->payload;

    char ssid_string[40] = {0};
    char mac_string[20] = {0};

    // Store the last time a packet was received from each MAC address
    // static uint32_t last_time_received[MAX_DEVICES] = {0};
    // static uint8_t mac_list[MAX_DEVICES][6];

    static packet_info_timeout mac_list[MAX_DEVICES];
    static int current_mac_index = 0;
    bool mac_found = false;

    // Check if it's a Probe Request frame (Management Frame with subtype 4)
    // first two bits are reserved for the version, not relevant
    // next two bits indicate the type of frame
    // last four bits indicate the subtype of the frame

    // We arent filtering for mngmt type because when we created the function we already do that with WIFI_PROMIS_FILTER_MASK_MGMT

    // fctl     XXXX XXXX 0100 XXXX  What fctl has to be in order to be accepted
    // 0x00F0   0000 0000 1111 0000  Mask that we use, we only care about specific bits to determine subtype
    // Result   0000 0000 0100 0000  Version 0 (00), Management frame (00), Subtype 4 (Probe Request)
    // https://howiwifi.com/2020/07/13/802-11-frame-types-and-formats/

    if ((header->fctl & MNGMT_PROBE_MASK) == MNGMT_PROBE_PACKET) // Frame Control: Type=0 (Management), Subtype=4 (Probe Request)
    {
        // The payload will contain the information we are looking for, only use that
        const uint8_t *payload = header->payload;
        uint8_t *ssid = NULL;
        size_t ssid_len = 0;

        // Parse the Probe Request frame to find the SSID
        for (size_t i = 0; i < sizeof(packet->payload) - sizeof(wifi_management_header);)
        {
            // Check the element ID for SSID
            if (payload[i] == 0) // Element ID for SSID is 0
            {
                ssid_len = payload[i + 1];         // Length of SSID
                ssid = (uint8_t *)&payload[i + 2]; // SSID starts after ID and Length
                break;
            }
            // Move to the next information element
            i += payload[i + 1] + 2; // Move to next element
        }

        // If the packet had an ssid
        if (ssid && ssid_len > 0)
        {
            snprintf(ssid_string, ssid_len + 1, "%.*s", ssid_len, ssid);

            // We only care about a specific SSID
            if (!strcmp(ssid_string, "Dog_Repel"))
            {
                uint8_t *mac_address = header->SA;

                // Get the current time in milliseconds
                struct timeval current_time;
                gettimeofday(&current_time, NULL);

                // seconds * 1000 = ms, microseconds / 1000 = ms
                uint32_t current_timestamp = current_time.tv_sec * 1000 + current_time.tv_usec / 1000;

                bool can_process = true;

                // Check if the MAC address is already in the list
                for (int i = 0; i < current_mac_index; i++)
                {
                    // Check for the same MAC address
                    if (!memcmp(mac_address, mac_list[i].mac, 6))
                    {
                        // ESP_LOGI(TAG, "[!] current_timestamp: %ld vs mac_list[i].last_time_received: %ld = %ld", current_timestamp, mac_list[i].last_time_received, current_timestamp - mac_list[i].last_time_received);

                        // Check if the cooldown period has expired
                        if (current_timestamp - mac_list[i].last_time_received < COOLDOWN_PERIOD)
                        {
                            can_process = false; // Don't process if still in cooldown
                        }
                        break;
                    }
                }

                // If it's okay to process the packet
                if (can_process)
                {
                    if (current_mac_index < MAX_DEVICES)
                    {
                        // First check if the mac already is in the list and update the timestamp value
                        for (int i = 0; i < current_mac_index; i++)
                        {
                            if (!memcmp(mac_address, mac_list[i].mac, 6))
                            {
                                ESP_LOGI(TAG, "[+] Updated mac in list with timestamp: %ld", current_timestamp);
                                mac_list[i].last_time_received = current_timestamp;
                                mac_found = true;
                            }
                        }

                        // If the MAC was not found in the list, add it to the list
                        if (!mac_found)
                        {
                            ESP_LOGI(TAG, "[+] Added mac to list with timestamp: %ld", current_timestamp);
                            memcpy(mac_list[current_mac_index].mac, mac_address, 6);
                            mac_list[current_mac_index].last_time_received = current_timestamp;
                            current_mac_index++;
                        }
                    }

                    // Format the MAC to a string so it can be printed and sent to the server as a string
                    formatMAC2STR(mac_address, mac_string);

                    // Log the MAC address
                    ESP_LOGI(TAG, "Device MAC Address: %s", mac_string);
                    ESP_LOGI(TAG, "Received RSSI: %d", packet->rx_ctrl.rssi);
                    ESP_LOGI(TAG, "Device attempted to connect to SSID: %.*s\n\n", ssid_len, ssid);

                    // Print all of the available stored MAC addresses
                    printMACS(mac_list, current_mac_index);
                }
            }
        }
    }
}

void printMACS(packet_info_timeout mac_list[MAX_DEVICES], int size)
{
    printf("All MACs saved: \n");
    for (int i = 0; i < size; i++)
    {
        printf("[%d]: ", i + 1);
        printMAC(mac_list[i].mac);
    }
    fflush(stdout);
}

void printMAC(uint8_t mac[6])
{
    printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

#define MAC_STR_LEN 18
void formatMAC2STR(uint8_t mac[6], char *returnMACstr)
{
    snprintf(returnMACstr, MAC_STR_LEN, "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2],
             mac[3], mac[4], mac[5]);
}

void delayMs(uint16_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}
