#ifndef BATTERY_H
#define BATTERY_H

#include <stdbool.h>

enum battery_state_e {
    CHARGING = '+',
    DISCHARGING = '-',
    FULL = ' ',
    UNKNOWN = '?'
};

struct battery_status_s {
    enum battery_state_e state;
    float level; 
};

bool get_battery_status(struct battery_status_s *status);

#endif // BATTERY_H
