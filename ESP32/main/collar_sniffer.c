
// #define COLLAR_CODE
#define STATION_CODE

/*
Todo:
Station code:
- add webpage to setup wifi connection (store in NVS)
- add camera code

Collar code:
- clean up code a bit
*/
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
#include "esp_mac.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"

#ifdef STATION_CODE
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
static bool runTCPtask = false;

#define WIFI_SSID "Ext_2.4"
#define WIFI_PASS "Moorparkcalifornia"
#define CONFIG_CHANNEL 6
#define MAC_STR_LEN 18

// Used to connect to TCP server
#define HOST_IP_ADDR "192.168.0.28" // localhost
#define PORT 2222

#define MNGMT_PROBE_MASK 0x00F0
#define MNGMT_PROBE_PACKET 0x0040
#define MAX_DEVICES 10
#define COOLDOWN_PERIOD 5000 // Cooldown period in milliseconds

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
static void tcp_client_task(void *pvParameters);
bool handleDeviceCommand(char *rx_buffer);
char **tokenizeString(char *str, const char *delim);
void wifi_init_sta();
void delayMs(uint16_t ms);
void printMACS(packet_info_timeout mac_list[MAX_DEVICES], int size);
void printMAC(uint8_t mac[6]);
void formatMAC2STR(uint8_t mac[6], char *returnMACstr);

// Used to store MAC of device when an event occurs
static char got_collar_mac[18] = {0};
static uint8_t station_mac_addr[6] = {0};
static char station_mac_addr_str[20] = {0};
static int8_t currentRSSI = 0;

void app_main(void)
{
    ESP_LOGI(TAG, "[+] Startup...");

    ESP_LOGI(TAG, "[+] Reading MAC of Station Mode");
    esp_read_mac(station_mac_addr, ESP_MAC_WIFI_STA);      // Read the MAC of the ESP32 Station
    formatMAC2STR(station_mac_addr, station_mac_addr_str); // Save the MAC as a string so it can be sent to the server

    ESP_LOGI(TAG, "[+] Initializing NVS storage");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_LOGI(TAG, "[+] Initializing ESP in Station Mode");
    wifi_init_sta();

    ESP_LOGI(TAG, "[+] Starting sniffing task...");
    xTaskCreate(&sniffer_task, "sniffer_task", 10000, NULL, 1, NULL);

    while (RUNNING)
    {
        delayMs(1);
    }
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
        ESP_LOGI(TAG, "[✓] Station started");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        runTCPtask = false;
        ESP_LOGI(TAG, "[!] Disconnected from Wi-Fi, trying to reconnect...");
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "[✓] Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        runTCPtask = true;

        // Create the TCP client task
        xTaskCreate(tcp_client_task, "tcp_client_task", 4096, NULL, 5, NULL);
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

    ESP_LOGI(TAG, "[✓] Initialization of Station finished.");
}

static void sniffer_task(void *pvParameter)
{
    ESP_LOGI(TAG, "[+] Sniffer Task created");
    wifi_sniffer_init();

    while (1)
    {
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
    ESP_LOGI(TAG, "[✓] Sniffer Task finished initialization");
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
                                // ESP_LOGI(TAG, "[!] Updated mac in list with timestamp: %ld", current_timestamp);
                                mac_list[i].last_time_received = current_timestamp;
                                mac_found = true;
                            }
                        }

                        // If the MAC was not found in the list, add it to the list
                        if (!mac_found)
                        {
                            memcpy(mac_list[current_mac_index].mac, mac_address, 6);
                            mac_list[current_mac_index].last_time_received = current_timestamp;
                            current_mac_index++;
                        }
                    }

                    // Format the MAC to a string so it can be printed and sent to the server as a string
                    formatMAC2STR(mac_address, mac_string);

                    // Save the RSSI so it can be sent to the server
                    currentRSSI = packet->rx_ctrl.rssi;

                    // Log the info obtained
                    ESP_LOGI(TAG, "[!] MAC Address: %s", mac_string);
                    ESP_LOGI(TAG, "[!] RSSI: %d", packet->rx_ctrl.rssi);
                    ESP_LOGI(TAG, "[!] SSID: %.*s\n\n", ssid_len, ssid);

                    // Save the stored MAC so that the TCP task can send a message
                    strcpy(got_collar_mac, mac_string);

                    // Print all of the available stored MAC addresses
                    printMACS(mac_list, current_mac_index);
                }
            }
        }
    }
}

void printMACS(packet_info_timeout mac_list[MAX_DEVICES], int size)
{
    printf("\n[!] All MACs saved: \n");
    for (int i = 0; i < size; i++)
    {
        printf("[%d]: ", i + 1);
        printMAC(mac_list[i].mac);
    }
    printf("\n");
    fflush(stdout);
}

void printMAC(uint8_t mac[6])
{
    printf("%02X:%02X:%02X:%02X:%02X:%02X\n",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

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

/*
TCP task will:
- communicate events (send MAC of device to server)
- receive settings adjustments for distance and camera activation

DogRepelEvent:MAC:StationID:Distance
*/
/* this will be used to send Login and Keep Alives, make it simpler */
static void tcp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char tx_buffer[128];
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

    while (runTCPtask)
    {
        static bool connected = false;

        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(host_ip);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;

        ESP_LOGI(TAG, "[!] Creating TCP socket");
        int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0)
        {
            ESP_LOGE(TAG, "[X] Unable to create socket: errno %d, retrying.", errno);
            delayMs(3000);
            continue;
        }
        ESP_LOGI(TAG, "[✓] Created TCP socket");

        // This is used to set the timeout in seconds so that the sends and recvs don't hang
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout);

        ESP_LOGI(TAG, "[!] Connecting to %s:%d", host_ip, PORT);

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in));
        if (err != 0)
        {
            ESP_LOGE(TAG, "[X] Socket unable to connect: errno %d. Closing socket.", errno);
            close(sock); // close the socket to prevent multiple sockets from opening
            delayMs(3000);
            continue;
        }
        ESP_LOGI(TAG, "[✓] Successfully connected");
        connected = true;

        // Change the timeout time now to 1 second
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout);

        while (connected)
        {
            if (strlen(got_collar_mac) > 0)
            {
                // Make sure to also send the station MAC along with the collar MAC
                snprintf(tx_buffer, sizeof(tx_buffer), "EVENT/%s/%s/%d", station_mac_addr_str, got_collar_mac, currentRSSI);
                err = send(sock, tx_buffer, strlen(tx_buffer), 0);

                // Clear the value so that it only sends once
                memset(got_collar_mac, 0, sizeof(got_collar_mac));
                currentRSSI = 0;
                if (err < 0)
                {
                    ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
                }
            }

            // In this case, the station is receiving a message to update its distance threshold
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (len > 0)
            {
                rx_buffer[len] = '\0'; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s: %s", len, host_ip, rx_buffer);

                // Pass what was received to be handled
                handleDeviceCommand(rx_buffer);
            }

            if (sock == -1)
            {
                ESP_LOGE(TAG, "[!] Socket failed, closing.");
                close(sock);
                break;
            }
            delayMs(1);
        }
    }
    vTaskDelete(NULL);
}

bool handleDeviceCommand(char *rx_buffer)
{
    // DogRepelEvent-StationMAC-distance

    // Process the received string to get tokens
    char **tokens = tokenizeString(rx_buffer, "-");

    printf("tokens: %s %s %s", tokens[1], tokens[2], tokens[3]);
    fflush(stdout);
    // // Compare the MAC of the device to ensure the message is for the device
    // if (!strcmp(, )
    // {
    //     ESP_LOGI(TAG, "Valid command has been received");
    //     // Reading
    //     if (!strcmp(tokens[2], "R"))
    //     {
    //         // Read the value from the LED
    //         if (!strcmp(tokens[3], "L"))
    //         {
    //             ESP_LOGI(TAG, "Reading the LEDs value...");
    //             valueLED = gpio_get_level(LED);
    //             snprintf(tx_buffer, sizeBuffer, "ACK:%d", valueLED);
    //             validOp = true;
    //         }

    //         // Read the value from the ADC
    //         else if (!strcmp(tokens[3], "A"))
    //         {
    //             ESP_LOGI(TAG, "Reading the ADCs value...");
    //             valueADC = ADC1_Ch3_Read();
    //             snprintf(tx_buffer, sizeBuffer, "ACK:%d", valueADC);
    //             validOp = true;
    //         }

    //         // Read the dev id from the device that was set
    //         else if (!strcmp(tokens[3], "DEV"))
    //         {
    //             nvs_handle_t my_handle;
    //             err = nvs_open("wifi_storage", NVS_READONLY, &my_handle);
    //             if (err != ESP_OK)
    //             {
    //                 ESP_LOGI(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    //             }
    //             else
    //             {
    //                 ESP_LOGI(TAG, "Attempting to retrieve\n");
    //                 // Retrieve dev id from NVS
    //                 err = nvs_get_str(my_handle, "receivedName", returnStorage, &returnStorage_len);

    //                 if (err != ESP_OK)
    //                 {
    //                     ESP_LOGI(TAG, "Failed to read dev ID from NVS: %s\n", esp_err_to_name(err));
    //                     snprintf(tx_buffer, sizeBuffer, "NACK");
    //                 }
    //                 else
    //                 {
    //                     snprintf(tx_buffer, sizeBuffer, "ACK:%s", returnStorage);
    //                     validOp = true;
    //                 }
    //             }
    //             nvs_close(my_handle);
    //         }
    //     }
    //     // Writing
    //     else if (!strcmp(tokens[2], "W"))
    //     {
    //         if (!strcmp(tokens[3], "L"))
    //         {
    //             if (!strcmp(tokens[4], "0"))
    //             {
    //                 ESP_LOGI(TAG, "Turning the LED off...");
    //                 gpio_set_level(LED, 0);
    //                 snprintf(tx_buffer, sizeBuffer, "ACK:%d", gpio_get_level(LED));
    //                 validOp = true;
    //             }
    //             else if (!strcmp(tokens[4], "1"))
    //             {
    //                 ESP_LOGI(TAG, "Turning the LED on...");
    //                 gpio_set_level(LED, 1);
    //                 snprintf(tx_buffer, sizeBuffer, "ACK:1");
    //                 validOp = true;
    //             }
    //         }

    //         // Facilitates NVS clearing, extra command to make it easier
    //     }
    //     else if (!strcmp(tokens[2], "RESET"))
    //     {
    //         // Erase everything that is in the nvs storage
    //         ESP_LOGI(TAG, "Erasing everything in NVS...");
    //         err = nvs_flash_erase();
    //         if (err == ESP_OK)
    //         {
    //             ESP_LOGI(TAG, "NVS erased! Rebooting.");
    //             esp_restart();
    //         }
    //     }
    // }

    // // If there was no valid operation, add a NACK as a response
    // if (!validOp)
    // {
    //     snprintf(tx_buffer, sizeBuffer, "NACK");
    //     return 0;
    // }
    return 1;
}

char **tokenizeString(char *str, const char *delim)
{
    char **tokens = (char **)malloc(30 * sizeof(char *));
    char *token;
    int i = 0;

    token = strtok(str, delim);
    while (token != NULL)
    {
        tokens[i++] = token;
        token = strtok(NULL, delim);
    }
    tokens[i] = NULL; // Null-terminate the tokens array

    return tokens;
}

#endif

#ifdef COLLAR_CODE

#include "esp_pm.h"
/* STA Configuration */
#define WIFI_STA_SSID "Dog_Repel"
#define WIFI_STA_PASSWD "123456789"
#define ESP_MAXIMUM_RETRY 5
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK

static const char *TAG_AP = "WiFi SoftAP";
static const char *TAG_STA = "WiFi Station";

// Function prototypes
esp_err_t initIO();
void ADC1_Ch3_Ini(void);
float ADC1_Ch3_Read(void);
float ADC1_Ch3_Read_mV(void);
bool handleDeviceCommand(char **tokens, char *tx_buffer, size_t sizeBuffer);
bool handleWifiMessages(char **tokens, char *tx_buffer, size_t sizeBuffer);
void delayMs(uint16_t ms);

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data);

void wifi_init_softap(void);
void wifi_init_sta(void);

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
        ESP_LOGI(TAG_STA, "Station started");
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG_STA, "Sending out probe request.");
        delayMs(5000);
        esp_wifi_connect();
    }
}

/* Initialize wifi station */
void wifi_init_sta(void)
{
    esp_netif_init();
    esp_event_loop_create_default();

    // Initialize Wi-Fi in station mode
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();

    esp_wifi_init(&wifi_initiation);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

    // Configure Wi-Fi station
    wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = WIFI_STA_SSID,
            .password = WIFI_STA_PASSWD,
            .scan_method = WIFI_FAST_SCAN, // this is used to save battery
            .failure_retry_cnt = ESP_MAXIMUM_RETRY,
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    // Apply the Wi-Fi configuration
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_sta_config);
    esp_wifi_start();

    ESP_LOGI(TAG_STA, "wifi_init_sta finished.");
}

void delayMs(uint16_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}
//---------------------------------------END OF WIFI-------------------------------------------

void app_main(void)
{
    esp_err_t err;
    err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    esp_pm_config_t power_cfg = {
        .max_freq_mhz = 80,
        .min_freq_mhz = 80,
        .light_sleep_enable = false,
    };

    esp_err_t result = esp_pm_configure((const void *)&power_cfg);
    if (result != ESP_OK)
    {
        ESP_LOGE("PM", "[X] Failed to configure power management: %s", esp_err_to_name(result));
    }

    wifi_init_sta();

    while (1)
    {
        delayMs(1);
    }
}

#endif