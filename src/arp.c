#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "arp.h"
#include "desktop-utils/macros.h"

struct ether_ip_s {
    union {
        uint8_t mac[ETH_ALEN];
        unsigned long lmac;
    };
    time_t last_changed;
    in_addr_t ip;
};

struct ether_ip_s table[1024];
size_t table_size = 0;


bool lookup_and_insert(const struct ether_ip_s *new);

bool check_arp_table(char *message, size_t len) {
    FILE *f;
    struct ether_ip_s tmp;
    char ip_address[32];
    char mac_address[32];

    snprintf(message, len, "ARP table OK");
    CHK_NULL(f = fopen("/proc/net/arp", "r"));
    time(&tmp.last_changed);
    fscanf(f, "%*[^\n]\n");
    while ( !feof(f) ) {
        CHK(fscanf(f, "%s%*[ ]0x%*x%*[ ]0x%*x%*[ ]%[a-f0-9:]%*[^\n]\n", ip_address, mac_address), != 2);
        CHK_NEG(inet_pton(AF_INET, ip_address, &tmp.ip));

        tmp.lmac = 0UL;
        CHK(sscanf(
            mac_address, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
            &tmp.mac[0], &tmp.mac[1], &tmp.mac[2], &tmp.mac[3], &tmp.mac[4], &tmp.mac[5]
        ), != 6);

        if ( ! lookup_and_insert(&tmp) ) {
            snprintf(message, len, "Possible MITM attack, please check %s", ip_address);
            break;
        }
    }
    SAFE_FREE(fclose, f);
    return true;

    fail:
    SAFE_FREE(fclose, f);
    snprintf(message, len, "ARP table ???");
    return false;
}

bool lookup_and_insert(const struct ether_ip_s *new) {
    for ( size_t i = 0; i < table_size; i++ ) {
        if ( table[i].ip == new->ip ) {
            if ( table[i].lmac != new->lmac ) {
                if ( table[i].last_changed + ALERT_TIMEOUT > new->last_changed ) {
                    return false;
                } else {
                    // Update the DB, it must be a new host
                    table[i].lmac = new->lmac;
                    table[i].last_changed = new->last_changed;
                    return true;
                }
            } else {
                // Update last seen
                table[i].last_changed = new->last_changed;
                return true;
            }
        }
    }

    if ( table_size < ARRAY_SIZE(table) ) {
        memcpy(&table[table_size], new, sizeof(struct ether_ip_s));
        table_size++;
    } else {
        // To big, let's restart from the begining
        table_size = 0;
        return false;
    }

    return true;
}
