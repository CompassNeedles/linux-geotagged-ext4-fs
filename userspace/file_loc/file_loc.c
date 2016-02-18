#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "file_loc.h"

int main(int argc, char **argv)
{
    struct gps_location loc = {.latitude = 0, .longitude = 0, .accuracy = 0, };
    long age;

    if (argc != 2) {
        printf("Usage: ./file_loc \"pathname\"\n");
        return -1;
    }
    printf("Pathname: %s.\n", argv[1]);
    errno = 0;
    age = get_gps_location(argv[1], &loc);
    if (age < 0) {
        printf("Error[%i]: %s\n", errno, strerror(errno)); /* From kernel. */
        return -1;
    } else if (loc.accuracy <= 0) {
        printf("Error: %s (accuracy is zero).\n", strerror(19));
        return -1;
    } else if (loc.latitude == 0 && loc.longitude == 0) {
        printf("Error: this cannot be New York.\n");
        return -1;
    } else if (age > 0x7fffffff) {
        printf("Error: should not overflow.\n");
        return -1;
    } else {
        printf("Latitude: %f\n", loc.latitude);
        printf("Longitude: %f\n", loc.longitude);
        printf("Accuracy: %f\n", loc.accuracy);
        printf("Age: %lu\n", (long unsigned)age);
        printf("URL: https://www.google.com/maps/@%f,%f,15z\n", loc.latitude,
            loc.longitude);
        return 0;
    }
}
