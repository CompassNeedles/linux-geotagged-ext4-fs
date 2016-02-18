#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "gpsd.h"

#define POLL_TIME 1000000 /* us */
#define DBG_P1 0

#if DBG_P1
    #define dbg(fmt, ...) printf("Gpsd: " fmt, ## __VA_ARGS__)
#else
    #define dbg(fmt, ...)
#endif

static int poll_location(void);

void daemon_mode(void)
{
    FILE *lg = NULL;
    pid_t pid;

    pid = fork();

    if (pid < 0) {
        perror("Failed to fork");
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
        exit(EXIT_SUCCESS);

    if (setsid() < 0) {
        perror("Failed to set sid");
        exit(EXIT_FAILURE);
    }

    lg = fopen(GPSD_LOG, "w");
    if (lg == NULL)
        perror("Failed to open gpsd log file");
    else {
        fprintf(lg, "GPS Daemon {%i}\n", getpid());
        fclose(lg);
    }
    close(0);
    close(1);
    close(2);
    chdir("/data/misc");
    umask(0);
}

static int poll_location(void)
{
    FILE *fp = NULL;
    FILE *lg = NULL;
    char *line = NULL;
    char *token, *endptr;
    size_t len = 0;
    ssize_t read;
    struct gps_location user_loc;
    int outcome;
    dbg("\n");

    fp = fopen(GPS_LOCATION_FILE, "r");
    if (!fp)
        goto read_error;

    read = getline(&line, &len, fp);
    if (read > 0) {
        /* latitude */
        token = strtok(line, " \t");
        if (!token)
            goto read_error;
        user_loc.latitude = strtod(token, &endptr);
        dbg("latitude:%f, 0x%llx, var size:%u\n", user_loc.latitude, 
            (long long int) user_loc.latitude, sizeof(user_loc.latitude));
    } else
        goto read_error;

    read = getline(&line, &len, fp);
    if (read > 0) {
        /* longitude */
        token = strtok(line, " \t");
        if (!token)
            goto read_error;
        user_loc.longitude = strtod(token, &endptr);
        dbg("longitude:%f, 0x%llx, var size:%u\n", user_loc.longitude, 
            (long long int) user_loc.longitude, sizeof(user_loc.longitude));
    } else
        goto read_error;

    read = getline(&line, &len, fp);
    if (read > 0) {
        /* accuracy */
        token = strtok(line, " \t");
        if (!token)
            goto read_error;
        user_loc.accuracy = strtof(token, &endptr);
        dbg("accuracy:%f, 0x%llx, var size:%u\n", user_loc.accuracy,
            (long long int) user_loc.accuracy, sizeof(user_loc.accuracy));
    } else
        goto read_error;

    free(line);
    dbg("\nwriting: %u, %p, %p, %p, %p\n", sizeof(user_loc), &user_loc,
        &user_loc.latitude, &user_loc.longitude, &user_loc.accuracy);
    outcome = set_gps_location(&user_loc);
    lg = fopen(GPSD_LOG, "w");
    if (outcome) {
        dbg("outcome:%i\n", outcome);
        if (lg) {
            fprintf(lg, "Error[%i]: %s\n", errno, strerror(errno));
            fclose(lg);
        }
    }
    return outcome;

read_error:
    lg = fopen(GPSD_LOG, "w");
    if (lg) {
        fprintf(lg, "Error[%i] reading: %s\n", errno, strerror(errno));
        fclose(lg);
    }
    fclose(fp);
    return -1;
}


int main(int argc, char *argv[])
{
#if !DBG_P1
    dbg("\nCreating daemon\n");
    daemon_mode();
#endif
    while (1) {
        poll_location();
        usleep(POLL_TIME);
    }
    return 0;
}
