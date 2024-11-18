#include <inttypes.h>
#include <stddef.h>

#include "lwip/sockets.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio.h"

#include "helper.h"
#include "wifi.h"

#define NVS_NAMESPACE_CONNECTION "connection_data"

extern bool runTCPtask;

void tcp_client_task(void *pvParameters);
bool handleDeviceCommand(char *rx_buffer);