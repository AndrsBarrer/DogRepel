#include "mac.h"
#include "wifi.h"
#include "config.h"

#ifdef STATION

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

static packet_info_timeout mac_list[MAX_DEVICES];
#endif