#ifndef ARP_H
#define ARP_H

#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#define ALERT_TIMEOUT   ((time_t)40L)

bool check_arp_table(char *message, size_t len);

#endif // ARP_H
