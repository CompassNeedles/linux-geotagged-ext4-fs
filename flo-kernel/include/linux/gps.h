#ifdef CONFIG_GPSFS
#ifndef __LINUX_GPS_H
#define __LINUX_GPS_H

struct gps_location {
    double  latitude;
    double  longitude;
    float   accuracy;  /* in meters */
};

#include <linux/timer.h>

int get_current_gps_location(struct gps_location *l);
int get_last_gps_update_time(struct timespec *t);

#endif

#endif
