#include "main.h"
#include "wifi.h"
#include "pwm.h"
#include "tcp.h"

#ifdef STATION

/* TAG of ESP32 for I/O operation */
static const char *TAG = "Sniff sniff";

// Declare a semaphore to control task execution
SemaphoreHandle_t xSemaphore;

void initIO()
{
    // The LEDs will be used to signal important events
    gpio_set_direction(ALARM_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(SERVER_DC_LED, GPIO_MODE_OUTPUT);
}

static void resolve_server_ip(char *host_ip, size_t ip_size)
{
    struct sockaddr_in broadcast_addr, source_addr;
    int udp_sock;
    char *message = "DISCOVER_SERVER";
    char receive_buffer[MAX_RECEIVE_BUFFER_SIZE];
    socklen_t addr_len = sizeof(source_addr);
    struct timeval timeout;
    int len;

    // Create a UDP socket
    while (1)
    {
        udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (udp_sock < 0)
        {
            ESP_LOGE(TAG, "[X] Unable to create UDP socket: errno %d", errno);
            vTaskDelay(pdMS_TO_TICKS(BROADCAST_INTERVAL_MS)); // Wait before retrying
            continue;
        }

        // Set the broadcast option
        int broadcast_enable = 1;
        if (setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0)
        {
            ESP_LOGE(TAG, "[X] setsockopt failed: errno %d", errno);
            close(udp_sock);
            vTaskDelay(pdMS_TO_TICKS(BROADCAST_INTERVAL_MS)); // Wait before retrying
            continue;
        }

        // Set receive timeout
        timeout.tv_sec = 0;
        timeout.tv_usec = RECEIVE_TIMEOUT_MS * 1000; // Set timeout to 2 seconds
        if (setsockopt(udp_sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
        {
            ESP_LOGE(TAG, "[X] setsockopt for timeout failed: errno %d", errno);
            close(udp_sock);
            vTaskDelay(pdMS_TO_TICKS(BROADCAST_INTERVAL_MS)); // Wait before retrying
            continue;
        }

        // Set up broadcast address
        memset(&broadcast_addr, 0, sizeof(broadcast_addr));
        broadcast_addr.sin_family = AF_INET;
        broadcast_addr.sin_port = htons(UDP_PORT);
        broadcast_addr.sin_addr.s_addr = inet_addr("255.255.255.255");

        // Send the broadcast message
        if (sendto(udp_sock, message, strlen(message), 0, (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr)) < 0)
        {
            ESP_LOGE(TAG, "[X] Error sending broadcast: errno %d", errno);
            close(udp_sock);
            vTaskDelay(pdMS_TO_TICKS(BROADCAST_INTERVAL_MS)); // Wait before retrying
            continue;
        }
        ESP_LOGI(TAG, "[!] Broadcast sent");

        // Wait for the server's response
        len = recvfrom(udp_sock, receive_buffer, sizeof(receive_buffer) - 1, 0, (struct sockaddr *)&source_addr, &addr_len);
        if (len < 0)
        {
            ESP_LOGE(TAG, "[X] recvfrom failed or timed out: errno %d", errno);
            close(udp_sock);
            vTaskDelay(pdMS_TO_TICKS(BROADCAST_INTERVAL_MS)); // Wait before retrying
            continue;
        }

        // Null-terminate the received data
        receive_buffer[len] = 0;

        // Log the received IP address and message
        ESP_LOGI(TAG, "[!] Received from %s: %s", inet_ntoa(source_addr.sin_addr), receive_buffer);

        // Store the server's IP address
        strncpy(host_ip, inet_ntoa(source_addr.sin_addr), ip_size);
        host_ip[ip_size - 1] = '\0'; // Ensure null termination

        // Close the UDP socket
        close(udp_sock);
        break; // Exit the loop once the server's IP is received
    }
}

void app_main(void)
{
    // Initialize LEDs for visual information
    initIO();

    // Initialize the server disconnected LED to ON
    gpio_set_level(SERVER_DC_LED, 1);

    ESP_LOGI(TAG, "[+] Startup...");

    ESP_LOGI(TAG, "[+] Reading MAC of Station Mode");
    esp_read_mac(station_mac_addr, ESP_MAC_WIFI_STA);      // Read the MAC of the ESP32 Station
    formatMAC2STR(station_mac_addr, station_mac_addr_str); // Save the MAC as a string so it can be sent to the server

    ESP_LOGI(TAG, "[+] Initializing NVS storage");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    nvs_handle_t my_handle;
    int err = nvs_open("wifi_storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        nvs_get_i8(my_handle, "currentWifiMode", (int8_t *)&currentWifiMode);
    }

    nvs_close(my_handle);

    if (currentWifiMode == WIFI_MODE_AP)
    {
        ESP_LOGI(TAG, "[+] Initializing ESP in Access Point Mode.");
        wifi_init_softap();
        ESP_LOGI(TAG, "[+] Initializing Web Server.");
        start_webserver();
    }
    else if (currentWifiMode == WIFI_MODE_STA)
    {
        ESP_LOGI(TAG, "[+] Initializing ESP in Station Mode");
        wifi_init_sta();

        resolve_server_ip(server_ip, sizeof(server_ip));
        ESP_LOGI(TAG, "[✓] Server IP resolved: %s", server_ip);

        // Prepare and set configuration of timer1 for low speed channels
        ledc_timer_config(&ledc_timer);

        ledc_channel_config(&ledc_channel); // Configure channel
        ledc_update_duty(LEDC_HS_MODE, LEDC_HS_CH1_CHANNEL);

        // Initialize fade service.
        // ledc_fade_func_install(0);

        // Initialize the semaphore as "available" (binary semaphore)
        xSemaphore = xSemaphoreCreateBinary();
        xSemaphoreGive(xSemaphore); // Set it to available initially

        ESP_LOGI(TAG, "[+] Starting sniffing task...");
        xTaskCreate(&sniffer_task, "sniffer_task", 10000, NULL, 1, NULL);
    }

    while (RUNNING)
    {
        // // Both SSID and Password were stored for the ESP to connect to given WiFi in Station mode
        if (storedSSID && storedPASS)
        {
            nvs_handle_t my_handle;
            err = nvs_open("wifi_storage", NVS_READWRITE, &my_handle);

            // Set integer in NVS storage so that next go around can be in Station mode
            err = nvs_set_i8(my_handle, "currentWifiMode", WIFI_MODE_STA);
            printf((err != ESP_OK) ? "Failed to set to Station Mode!\n" : "Set to Station mode.\n");
            nvs_close(my_handle);

            printf("Restarting in 5 seconds...");
            fflush(stdout);
            delayMs(5000);

            // Restart so that AP mode can be set
            esp_restart();
        }
        delayMs(1);
    }
}

// Define the task
void led_duty_task(void *param)
{
    // Set the duty cycle to 4000
    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 1);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

    // Turn the alarm LED on
    // ESP_LOGI("LED_TASK", "Turning ALARM LED ON");
    gpio_set_level(ALARM_LED, 1);

    // Keep the duty cycle at 4000 for 5 seconds
    vTaskDelay(pdMS_TO_TICKS(5000));

    // Turn off the duty cycle (set to 0)
    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 0);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

    // Turn the alarm LED off
    // ESP_LOGI("LED_TASK", "Turning ALARM LED OFF");
    gpio_set_level(ALARM_LED, 0);

    // Release the semaphore once the task completes
    xSemaphoreGive(xSemaphore);

    // Delete the task after it's done
    vTaskDelete(NULL);
}

void start_pwm_task()
{
    // Try to take the semaphore
    if (xSemaphoreTake(xSemaphore, (TickType_t)0) == pdTRUE)
    {
        // Create the task if the semaphore was successfully taken
        xTaskCreate(led_duty_task, "led_duty_task", 2048, NULL, 5, NULL);
    }
    else
    {
        // Task is already running, do nothing
        printf("Task is already running, skipping creation.\n");
    }
}

int32_t distanceAllowed = 0;

static void sniffer_task(void *pvParameter)
{
    ESP_LOGI(TAG, "[+] Sniffer Task created");
    wifi_sniffer_init();

    while (1)
    {
        // Get the distance value that should be allowed
        esp_err_t err = get_connection_type_int("distanceLabel", &distanceAllowed);
        if (err != ESP_OK)
        {
            // Handle error
            ESP_LOGE(TAG, "[X] Error retrieving Distance Allowed value.");
        }

        delayMs(10000);
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
                    // If the gotten rssi signal is less than the allowed,
                    // that means that the signal is exceeding the limit (we are working with negative values)
                    // https://www.metageek.com/training/resources/understanding-rssi/
                    if ((int8_t)packet->rx_ctrl.rssi >= (int8_t)distanceAllowed)
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
                        // Trigger the transducer to turn on
                        start_pwm_task();

                        // Format the MAC to a string so it can be printed and sent to the server as a string
                        formatMAC2STR(mac_address, mac_string);

                        // Save the RSSI so it can be sent to the server
                        currentRSSI = packet->rx_ctrl.rssi;

                        // Log the info obtained
                        ESP_LOGI(TAG, "[!] MAC Address: %s", mac_string);
                        ESP_LOGI(TAG, "[!] RSSI: %d", packet->rx_ctrl.rssi);
                        // ESP_LOGI(TAG, "[!] SSID: %.*s\n\n", ssid_len, ssid);

                        // Save the stored MAC so that the TCP task can send a message
                        strcpy(got_collar_mac, mac_string);

                        // Print all of the available stored MAC addresses
                        // printMACS(mac_list, current_mac_index);
                    }
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

#endif

#ifdef COLLAR

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
        .light_sleep_enable = true,
    };

    esp_err_t result = esp_pm_configure((const void *)&power_cfg);
    if (result != ESP_OK)
    {
        ESP_LOGE("PM", "[X] Failed to configure power management: %s", esp_err_to_name(result));
    }

    wifi_init_sta();
    esp_sleep_enable_timer_wakeup(5 * 1000000); // added 5 second sleep

    esp_deep_sleep_start();
    while (1)
    {
        delayMs(1);
    }
}

#endif