#include <X11/extensions/scrnsaver.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "desktop-utils/desktop-utils.h"
#include "desktop-utils/macros.h"

#define DEFAULT_INACTIVITY      "10m"
#define BUFFER_SIZE             256

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
bool get_datetime(char *dst, size_t len);
void set_status(Display *dpy, const char *str);


int main(int argc, char *const argv[], char *const envp[]) {
    Display *dpy = NULL;
    char datetime[BUFFER_SIZE];
    struct battery_status_s status;
    char dwm_status[BUFFER_SIZE];
    int ret = EXIT_SUCCESS;

	UNUSED(argc);
	UNUSED(argv);
	UNUSED(envp);

    CHK_FALSE(daemonize_uniq(LOCK_FILENAME));
    CHK_NULL(dpy = XOpenDisplay(NULL));

    while ( true ) {
        CHK_NEG(get_battery_status(&status));
        CHK_NEG(get_datetime(datetime, sizeof(datetime)));
        snprintf(dwm_status, sizeof(dwm_status), "BAT:%c%5.1f%% | %s", status.state, status.level, datetime);
        set_status(dpy, dwm_status);
        sleep(1);
    }
    ret = EXIT_SUCCESS;

    fail:
    SAFE_FREE(XCloseDisplay, dpy);
    return ret;
}

bool get_battery_status(struct battery_status_s *status) {
    FILE *f = NULL;
    unsigned long energy_now;
    unsigned long energy_full;
    char str_state[BUFFER_SIZE];

    CHK_NULL(f = fopen("/sys/class/power_supply/BAT0/energy_now", "r"));
    CHK(fscanf(f, "%lu", &energy_now), != 1);
    CHK_NEG(fclose(f));

    CHK_NULL(f = fopen("/sys/class/power_supply/BAT0/energy_full", "r"));
    CHK(fscanf(f, "%lu", &energy_full), != 1);
    CHK_NEG(fclose(f));

    CHK_NULL(f = fopen("/sys/class/power_supply/BAT0/status", "r"));
    CHK(fscanf(f, "%" STRINGIFY(BUFFER_SIZE) "s", str_state), != 1);
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

bool get_datetime(char *dst, size_t len) {
    time_t now;
    struct tm *tm;

    now = time(NULL);
    CHK_NULL(tm = localtime(&now));
    CHK(strftime(dst, len, "%a %d/%m/%Y %T", tm), == 0);

    return true;

    fail:
    *dst = 0;
    return false;
}

void set_status(Display *dpy, const char *str) {
    XStoreName(dpy, DefaultRootWindow(dpy), str);
    XSync(dpy, False);
}
