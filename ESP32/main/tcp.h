#include <inttypes.h>
#include <stddef.h>
#include "lwip/sockets.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio.h"
#include "helper.h"
#include "wifi.h"

#define NVS_NAMESPACE_CONNECTION "connection_data"
#define DEFAULT_ALLOWED_DISTANCE 50 // Define a default value if not set in NVS

extern bool runTCPtask;

/**
 * @brief TCP client task to connect to the server, send and receive data.
 * 
 * This function is responsible for establishing a TCP connection with the server, sending the MAC address of 
 * the ESP32 device to the server, and continuously sending events (such as collar MAC and RSSI values). It also 
 * listens for incoming messages from the server to update configuration or perform other tasks.
 * 
 * The task will keep the connection open and handle events as long as the connection is active. If the 
 * connection is lost or any error occurs, it will attempt to reconnect.
 * 
 * - Sends MAC address to the server to track the connected device.
 * - Periodically sends data like collar MAC and RSSI to the server.
 * - Handles incoming server messages and updates configuration accordingly.
 * 
 * @param pvParameters Parameters passed during task creation (not used in this function).
 */

void tcp_client_task(void *pvParameters);

/**
 * @brief Retrieve an integer value from the NVS (Non-Volatile Storage) for a given key.
 * 
 * This function opens the NVS handle, reads the integer value associated with the given key, and stores it
 * in the provided pointer. If the key is not found, a default value is assigned. If any error occurs during 
 * the process, an appropriate error message is logged.
 * 
 * - Opens the NVS handle for reading and writing.
 * - Retrieves the integer value corresponding to the key.
 * - If the key is not found, sets a default value.
 * - Closes the NVS handle after reading or handling errors.
 * 
 * @param key The key associated with the value to retrieve.
 * @param value Pointer to store the retrieved value.
 * @return esp_err_t The result of the operation, ESP_OK on success.
 */
esp_err_t get_connection_type_int(const char *key, int *value);

/**
 * @brief Set an integer value in the NVS (Non-Volatile Storage) for a given key.
 * 
 * This function opens the NVS handle, sets the integer value associated with the given key, and commits 
 * the change to the NVS. If any error occurs during the process, an appropriate error message is logged.
 * 
 * - Opens the NVS handle for reading and writing.
 * - Sets the integer value for the provided key.
 * - Commits the changes to NVS to make them persistent.
 * - Closes the NVS handle after writing or handling errors.
 * 
 * @param key The key for which the value is to be set.
 * @param value The integer value to set.
 * @return esp_err_t The result of the operation, ESP_OK on success.
 */
esp_err_t set_connection_type_int(const char *key, int value);

/**
 * @brief Handles incoming device commands, processes them, and updates NVS if necessary.
 * 
 * This function processes the received command in `rx_buffer`, which is expected to be in a specific format 
 * (e.g., "DogRepel/Operation/-30"). It tokenizes the string and based on the tokens, it performs actions 
 * such as writing or updating a distance value in the NVS (Non-Volatile Storage).
 * 
 * - Tokenizes the received string into components.
 * - Checks if the command is a "Write" or "Update" operation for a distance value.
 * - If the operation is "Write", it checks if the value already exists in NVS. If not, it writes the value.
 * - If the operation is "Update", it updates the existing value in NVS.
 * - Frees dynamically allocated memory for the tokens after processing.
 * 
 * @param rx_buffer The received command string to be processed.
 * @return esp_err_t The result of the operation, ESP_OK on success.
 */
esp_err_t handleDeviceCommand(char *rx_buffer);