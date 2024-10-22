#include "tcp.h"

static char *TAG = "TCP";

bool runTCPtask = false;

void tcp_client_task(void *pvParameters)
{
    char rx_buffer[128];
    char tx_buffer[128];
    int addr_family = 0;
    int ip_protocol = 0;

    while (runTCPtask)
    {
        static bool connected = false;

        // Advise that the esp is currently not connected to server
        gpio_set_level(SERVER_DC_LED, 1);

        struct sockaddr_in dest_addr;
        dest_addr.sin_addr.s_addr = inet_addr(server_ip);

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

        // ESP_LOGI(TAG, "[!] Connecting to %s:%d", host_ip, PORT);
        ESP_LOGI(TAG, "[!] Connecting to %s:%d", server_ip, PORT);

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
        gpio_set_level(SERVER_DC_LED, 0);

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
            if (len == 0)
            {
                ESP_LOGI(TAG, "Connection closed by the server.");
                close(sock);
                connected = false;
                break;
            }
            else if (len > 0)
            {
                rx_buffer[len] = '\0'; // Null-terminate whatever we received and treat like a string
                ESP_LOGI(TAG, "Received %d bytes from %s: %s", len, server_ip, rx_buffer);

                // Pass what was received to be handled
                handleDeviceCommand(rx_buffer);
            }
            else if (len < 0)
            {
                if (errno != EWOULDBLOCK && errno != EAGAIN)
                {
                    ESP_LOGE(TAG, "[X] Error occurred during receiving: errno %d. Closing socket.", errno);
                    close(sock);
                    connected = false;
                    break;
                }
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
