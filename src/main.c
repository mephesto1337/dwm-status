#include <X11/extensions/scrnsaver.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "arp.h"
#include "battery.h"
#include "volume.h"
#include "temperature.h"
#include "config.h"
#include "desktop-utils/desktop-utils.h"
#include "desktop-utils/macros.h"

#define BUFFER_SIZE             512

bool get_battery_status(struct battery_status_s *status);
bool get_datetime(char *dst, size_t len);
void set_status(Display *dpy, const char *str);


int main(int argc, char *const argv[], char *const envp[]) {
    Display *dpy = NULL;
    char datetime[BUFFER_SIZE];
    char arp_status[BUFFER_SIZE];
    struct battery_status_s status;
    char dwm_status[BUFFER_SIZE];
    long vol;
    double temp;
    int ret = EXIT_SUCCESS;

	UNUSED(argc);
	UNUSED(argv);
	UNUSED(envp);

    CHK_FALSE(daemonize_uniq(LOCK_FILENAME));
    CHK_NULL(dpy = XOpenDisplay(NULL));

    while ( true ) {
        CHK_FALSE(get_battery_status(&status));
        CHK_FALSE(get_datetime(datetime, sizeof(datetime)));
        CHK_FALSE(check_arp_table(arp_status, sizeof(arp_status)));
        CHK_FALSE(get_volume(&vol));
        CHK_FALSE(get_temperature(&temp));
        snprintf(dwm_status, sizeof(dwm_status), "%s | vol:%02ld%% | temp:%.1f°C | BAT:%c%5.1f%% | %s", arp_status, vol, temp, status.state, status.level, datetime);
        set_status(dpy, dwm_status);
        sleep(1);
    }
    ret = EXIT_SUCCESS;

    fail:
    SAFE_FREE(XCloseDisplay, dpy);
    return ret;
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
