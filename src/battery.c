#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "battery.h"
#include "desktop-utils/macros.h"

struct battery_status_files_s {
    const char *now;
    const char *full;
    const char *status;
};

const struct battery_status_files_s battery_status_files[] = {
    {
        .now = "/sys/class/power_supply/BAT0/energy_now",
        .full = "/sys/class/power_supply/BAT0/energy_full",
        .status = "/sys/class/power_supply/BAT0/status",
    },
    {
        .now = "/sys/class/power_supply/BAT0/charge_now",
        .full = "/sys/class/power_supply/BAT0/charge_full",
        .status = "/sys/class/power_supply/BAT0/status",
    },
    { .now = NULL, .full = NULL, .status = NULL },
};

bool get_battery_status(struct battery_status_s *status) {
    FILE *f = NULL;
    unsigned long energy_now;
    unsigned long energy_full;
    static const struct battery_status_files_s *files = NULL;
    char str_state[64];

    if ( files == NULL ) {
        for ( size_t i = 0; i < ARRAY_SIZE(battery_status_files); i++ ) {
            files = &battery_status_files[i];
            if ( access(files->now, R_OK) == 0 && access(files->full, R_OK) == 0 && access(files->status, R_OK) == 0 ) {
                break;
            }
        }
        if ( files->now == NULL || files->full == NULL || files->status == NULL ) {
            files = NULL;
            goto fail;
        }
    }

    CHK_NULL(f = fopen("/sys/class/power_supply/BAT0/energy_now", "r"));
    CHK(fscanf(f, "%lu", &energy_now), != 1);
    CHK_NEG(fclose(f));

    CHK_NULL(f = fopen("/sys/class/power_supply/BAT0/energy_full", "r"));
    CHK(fscanf(f, "%lu", &energy_full), != 1);
    CHK_NEG(fclose(f));

    CHK_NULL(f = fopen("/sys/class/power_supply/BAT0/status", "r"));
    CHK(fscanf(f, "%" STRINGIFY(sizeof(str_state)) "s", str_state), != 1);
    CHK_NEG(fclose(f));
    if ( strcasecmp(str_state, STRINGIFY(DISCHARGING)) == 0 ) {
        status->state = DISCHARGING;
    } else if ( strcasecmp(str_state, STRINGIFY(CHARGING)) == 0 ) {
        status->state = CHARGING;
    } else if ( strcasecmp(str_state, STRINGIFY(FULL)) == 0 ) {
        status->state = FULL;
    } else {
        status->state = UNKNOWN;
    }
    status->level = ((float)energy_now * 100.0f) / (float)energy_full;

    return true;

    fail:
    SAFE_FREE(fclose, f);
    return false;
}


