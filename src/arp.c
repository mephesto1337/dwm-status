#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "desktop-utils/macros.h"

struct ether_ip_s {
    uint8_t mac[ETH_ALEN];
    in_addr_t ip;
    bool has_changed;
};

struct ether_ip_s table[1024];
size_t table_size = 0;


bool mac_address_cmp(const uint8_t *mac1, const uint8_t *mac2);
void mac_address_copy(uint8_t *dst, const uint8_t *src);
bool lookup_and_insert(const struct ether_ip_s *new);

bool check_arp_table(char *message, size_t len) {
    FILE *f;
    size_t i;
    struct ether_ip_s tmp;
    char ip_address[32];
    char mac_address[32];
    char sep;
    int n;
    size_t off;

    snprintf(message, len, "ARP table OK");
    CHK_NULL(f = fopen("/proc/net/arp", "r"));
    fscanf(f, "%*[^\n]\n");
    while ( !feof(f) ) {
        CHK(fscanf(f, "%s%*[ ]0x%*x%*[ ]0x%*x%*[ ]%[a-f0-9:]%*[^\n]\n", ip_address, mac_address), != 2);
        CHK_NEG(inet_pton(AF_INET, ip_address, &tmp.ip));
        for ( off = 0, i = 0; i < ETH_ALEN; i++ ) {
            n = sscanf(&mac_address[off], "%02hhx%c", &tmp.mac[i], &sep);
            if ( n == 0 ) {
                break;
            } else if ( n == 1 ) {
                off += 2;
            } else if ( n == 2 ) {
                off += 3;
            } else {
                // WTF ???
                goto fail;
            }
        }
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

inline void mac_address_copy(uint8_t *dst, const uint8_t *src) {
    memcpy(dst, src, ETH_ALEN);
}

inline bool mac_address_cmp(const uint8_t *mac1, const uint8_t *mac2) {
    for ( size_t i = 0; i < ETH_ALEN; i++ ) {
        if ( mac1[i] != mac2[i] ) {
            return false;
        }
    }
    return true;
}

bool lookup_and_insert(const struct ether_ip_s *new) {
    for ( size_t i = 0; i < table_size; i++ ) {
        if ( table[i].ip == new->ip ) {
            if ( mac_address_cmp(table[i].mac, new->mac) ) {
                table[i].has_changed = false;
                return true;
            } else {
                mac_address_copy(table[i].mac, new->mac);
                if ( table[i].has_changed ) {
                    return false;
                } else {
                    return true;
                }
            }
        }
    }

    if ( table_size < ARRAY_SIZE(table) ) {
        memcpy(&table[table_size], new, sizeof(struct ether_ip_s));
        table[table_size].has_changed = false;
        table_size++;
    } else {
        exit(EXIT_FAILURE);
    }

    return true;
}
