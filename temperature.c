#include <stdio.h>
#include <stdbool.h>

#include "temperature.h"
#include "desktop-utils/macros.h"

bool get_temperature(double *temp) {
    bool ret = false;
    FILE *f = NULL;
    unsigned long _temp;

    CHK_NULL(f = fopen("/sys/devices/platform/coretemp.0/hwmon/hwmon2/temp1_input", "r"));
    CHK(fscanf(f, "%ld", &_temp), != 1);
    *temp = _temp / 1000.;
    ret = true;

    fail:
    SAFE_FREE(fclose, f);
    return ret;
}
