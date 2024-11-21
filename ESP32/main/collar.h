#include "freertos/FreeRTOS.h"
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

/**
 * @brief Structure representing a WiFi management frame header.
 *
 * This structure defines the header of a WiFi management frame, which includes fields such as
 * frame control, duration, addresses (source, destination, and BSS ID), sequence control,
 * and the data payload. The packed attribute ensures that the structure is packed without
 * padding, preserving the exact layout of the WiFi frame header.
 *
 * @param fctl Frame control, indicating the type and subtype of the frame.
 * @param duration Duration ID, used for time allocation in transmission management.
 * @param DA Destination address (MAC address of the intended receiver).
 * @param SA Sender address (MAC address of the sending device).
 * @param BSS_ID MAC address of the Access Point (AP) in the WiFi network.
 * @param seq_ctl Sequence control, used for managing frame sequencing and acknowledgment.
 * @param payload Data payload of the WiFi frame.
 */
typedef struct
{
    int16_t fctl;
    int16_t duration;
    uint8_t DA[6];
    uint8_t SA[6];
    uint8_t BSS_ID[6];
    int16_t seq_ctl;
    unsigned char payload[];
} __attribute__((packed)) wifi_management_header;

/**
 * @brief Structure to store information about a device's MAC address and the last time it was received.
 *
 * This structure holds the MAC address of a device and the timestamp of the last time the device's MAC address
 * was received. It is used to manage and track devices within a specified time period for further processing.
 *
 * @param mac The MAC address of the device (6 bytes).
 * @param last_time_received The last time the device's MAC address was recorded, in milliseconds.
 */
typedef struct
{
    uint8_t mac[6];              // Store the MAC of the device
    uint32_t last_time_received; // Store the last time the device mac was read
} packet_info_timeout;

/**
 * @brief Initializes the sniffer task and periodically checks the allowed distance value.
 *
 * This function creates the sniffer task, initializes the Wi-Fi sniffer, and enters a loop where it periodically
 * retrieves the "distanceAllowed" value from the configuration. If the value is successfully retrieved, it proceeds
 * to delay for 10 seconds before checking the value again. The task runs indefinitely, ensuring the sniffer is active
 * and the allowed distance is regularly updated.
 *
 * @param pvParameter A pointer to any parameters passed to the task. Currently unused.
 *
 * @note This task is designed to run in a FreeRTOS environment where the sniffer operates in the background,
 * periodically checking configuration values.
 */
static void sniffer_task(void *pvParameter);

/**
 * @brief Initializes the Wi-Fi sniffer in promiscuous mode.
 *
 * This function configures the Wi-Fi interface to operate in promiscuous mode, where it can receive all
 * Wi-Fi packets, including those not addressed to the device. It sets the country code to Mexico ("MX") and
 * configures the Wi-Fi channel range for the region. It also sets a filter to capture only management packets
 * (such as Probe Requests) and assigns the `wifi_sniffer_packet_handler` as the callback function to handle
 * the received packets.
 *
 * @note This function requires the ESP32 Wi-Fi driver to be initialized before use.
 */
static void wifi_sniffer_init(void);

/**
 * @brief Handles incoming Wi-Fi sniffed packets and processes Probe Request frames.
 *
 * This function is called whenever a packet is received in promiscuous mode by the Wi-Fi sniffer.
 * It checks if the packet is a Probe Request frame, extracts the SSID, and processes it if the SSID
 * matches a predefined string ("Dog_Repel"). The function then extracts the MAC address and checks if
 * it has been seen recently, respecting a cooldown period. If the packet's RSSI value is within an acceptable
 * range, it triggers the transducer, logs the MAC and RSSI, and stores the MAC address for later transmission.
 *
 * @param buff A pointer to the raw packet buffer.
 * @param type The type of the received Wi-Fi packet.
 */
static void wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);

/**
 * @brief Prints the MAC addresses saved in the provided list.
 *
 * This function iterates through the provided `mac_list` of `packet_info_timeout`
 * structures and prints the MAC addresses stored in the `mac` field of each element.
 * It prints a numbered list of MAC addresses, one per line.
 *
 * @param mac_list The list of packet information containing MAC addresses to be printed.
 * @param size The number of devices in the list to print.
 */
void printMACS(packet_info_timeout mac_list[MAX_DEVICES], int size);

/**
 * @brief Prints a single MAC address in hexadecimal format.
 *
 * This function prints a single MAC address in the standard format:
 * `XX:XX:XX:XX:XX:XX`, where `X` represents a hexadecimal digit.
 *
 * @param mac The MAC address to be printed, represented as an array of 6 bytes.
 */
void printMAC(uint8_t mac[6]);

/**
 * @brief Formats a MAC address into a string representation.
 *
 * This function formats a given MAC address into a human-readable string
 * representation in the format `XX:XX:XX:XX:XX:XX`, and stores it in the provided
 * `returnMACstr` buffer.
 *
 * @param mac The MAC address to be formatted, represented as an array of 6 bytes.
 * @param returnMACstr The output buffer where the formatted MAC address string will be stored.
 */
void formatMAC2STR(uint8_t mac[6], char *returnMACstr);

#ifdef COLLAR
#include "esp_pm.h"
#include "esp_sleep.h"

/* STA Configuration */
#define ESP_MAXIMUM_RETRY 5
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK

esp_err_t initIO();
#endif