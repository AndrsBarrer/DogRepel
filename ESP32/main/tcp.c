#include "config.h"
#include "tcp.h"

#ifdef STATION

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

        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

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

        // Send mac so that server can keep track of current device connected
        snprintf(tx_buffer, sizeof(tx_buffer), "MAC/%s", station_mac_addr_str);
        err = send(sock, tx_buffer, strlen(tx_buffer), 0);

        timeout.tv_sec = 0;
        timeout.tv_usec = 500000;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

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
                // ESP_LOGI(TAG, "[!] Received %d bytes from %s: %s", len, server_ip, rx_buffer);

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

esp_err_t get_connection_type_int(const char *key, int *value)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE_CONNECTION, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open NVS handle: %s", esp_err_to_name(err));
        return err;
    }

    // Get the value from NVS
    err = nvs_get_i32(my_handle, key, value);
    if (err != ESP_OK)
    {
        if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            ESP_LOGW(TAG, "[!] %s not set, using default value", key);
            *value = DEFAULT_ALLOWED_DISTANCE; // Set a default value if not found
        }
        else
        {
            ESP_LOGE(TAG, "Failed to get %s value, err: %s", key, esp_err_to_name(err));
        }
        nvs_close(my_handle); // Ensure handle is closed even if error occurs
        return err;
    }

    // Close the NVS handle
    nvs_close(my_handle);
    return ESP_OK;
}

// Helper function to open, write, commit, and close the NVS handle in one go
esp_err_t set_connection_type_int(const char *key, int value)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE_CONNECTION, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to open NVS handle: %s", esp_err_to_name(err));
        nvs_close(my_handle);
        return err;
    }

    // Set the value in NVS
    err = nvs_set_i32(my_handle, key, value);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set %d value, err: %s", value, esp_err_to_name(err));
        nvs_close(my_handle);
        return err;
    }

    // Commit the change
    err = nvs_commit(my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to commit %d value, err: %s", value, esp_err_to_name(err));
        nvs_close(my_handle);
        return err;
    }
    ESP_LOGI(TAG, "[✓] Updated distance value to %d", value);
    // Close the NVS handle
    nvs_close(my_handle);
    return ESP_OK;
}

esp_err_t handleDeviceCommand(char *rx_buffer)
{
    int err;
    nvs_handle_t my_handle;
    int32_t existing_value;

    // DogRepel/Operation/-30
    // Process the received string to get tokens
    char **tokens = tokenizeString(rx_buffer, "/");

    if (!tokens)
    {
        printf("[X] Failed to tokenize string.\n");
        return ESP_ERR_INVALID_ARG;
    }

    if (!strcmp(tokens[0], "DogRepel"))
    {
        int distanceValue = atoi(tokens[2]);

        if (!strcmp(tokens[1], "Write"))
        {
            // Check if value exists in NVS
            err = nvs_open(NVS_NAMESPACE_CONNECTION, NVS_READONLY, &my_handle);
            if (err == ESP_OK)
            {
                err = nvs_get_i32(my_handle, "distanceLabel", &existing_value);
                nvs_close(my_handle);

                if (err == ESP_OK)
                {
                    // Value exists, don't overwrite
                    ESP_LOGI(TAG, "[!] Distance value already exists in NVS: %ld", existing_value);
                    free(tokens);
                    return ESP_OK;
                }
                else if (err == ESP_ERR_NVS_NOT_FOUND)
                {
                    // Value doesn't exist, write it
                    err = set_connection_type_int("distanceLabel", distanceValue);
                    if (err != ESP_OK)
                    {
                        ESP_LOGE(TAG, "[X] Error setting initial distance allowed");
                    }
                    free(tokens);
                    return ESP_OK;
                }
            }
        }
        else if (!strcmp(tokens[1], "Update"))
        {
            err = set_connection_type_int("distanceLabel", distanceValue);
            if (err != ESP_OK)
            {
                ESP_LOGE(TAG, "[X] Error updating distance allowed");
            }
        }
    }
    // Free memory if tokens were dynamically allocated
    free(tokens);
    return ESP_OK;
}
#endif