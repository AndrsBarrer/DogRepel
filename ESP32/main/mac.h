#include "inttypes.h"
#include "wifi.h"

#define MAX_DEVICES 10

void printMACS(packet_info_timeout mac_list[MAX_DEVICES], int size);
void printMAC(uint8_t mac[6]);
void formatMAC2STR(uint8_t mac[6], char *returnMACstr);
