#ifndef _FILE_LOC_H_
#define _FILE_LOC_H_
/*
 * file_loc.h
 *
 * Columbia University
 * COMS W4118 Fall 2015
 * Homework 6 - Geo Tagged File System
 *
 */

#include <sys/syscall.h>

#define __NR_set_gps_location 378
#define __NR_get_gps_location 379

struct gps_location {
	double latitude;
	double longitude;
	float  accuracy;
};

static inline int set_gps_location(struct gps_location *loc)
{
	return syscall(__NR_set_gps_location, loc);
}

static inline int
get_gps_location(const char *pathname, struct gps_location *loc)
{
	return syscall(__NR_get_gps_location, pathname, loc);
}

#endif
