#include "wewe_net.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// A simplified implementation to get the gateway MAC address from /proc/net/route and /proc/net/arp
char* get_default_gateway_mac() {
    FILE *f;
    char line[256];
    char gateway_ip[16] = {0};
    char *result_mac = NULL;

    // 1. Find default gateway IP from /proc/net/route
    f = fopen("/proc/net/route", "r");
    if (f == NULL) {
        return NULL;
    }
    while (fgets(line, sizeof(line), f)) {
        char iface[16], dest[16], gateway[16], flags[16], refcnt[16], use[16], metric[16], mask[16];
        // A more robust parser would be better, but this works for standard route output
        if (sscanf(line, "%15s\t%15s\t%15s\t%15s\t%15s\t%15s\t%15s\t%15s", iface, dest, gateway, flags, refcnt, use, metric, mask) >= 4) {
            if (strcmp(dest, "00000000") == 0 && (strtol(flags, NULL, 16) & 2)) { // UG_FLAG (Gateway)
                // Convert hex IP to dotted decimal
                unsigned int ip_hex;
                sscanf(gateway, "%X", &ip_hex);
                sprintf(gateway_ip, "%u.%u.%u.%u",
                        (ip_hex & 0xFF),
                        (ip_hex >> 8) & 0xFF,
                        (ip_hex >> 16) & 0xFF,
                        (ip_hex >> 24) & 0xFF);
                break;
            }
        }
    }
    fclose(f);

    if (strlen(gateway_ip) == 0) {
        return NULL; // Gateway not found
    }

    // 2. Find MAC for gateway IP in /proc/net/arp
    f = fopen("/proc/net/arp", "r");
    if (f == NULL) {
        return NULL;
    }
    // Skip header line
    fgets(line, sizeof(line), f);
    while (fgets(line, sizeof(line), f)) {
        char ip_addr[16], mac_addr[20], device[16];
        if (sscanf(line, "%15s %*s %*s %19s %*s %15s", ip_addr, mac_addr, device) == 3) {
            if (strcmp(ip_addr, gateway_ip) == 0) {
                result_mac = strdup(mac_addr);
                break;
            }
        }
    }
    fclose(f);

    return result_mac;
}
