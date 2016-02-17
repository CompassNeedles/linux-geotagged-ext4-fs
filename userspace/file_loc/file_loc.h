#ifndef _FILE_LOC_H_
#define _FILE_LOC_H_
/*
 * file_loc.h
 *
 * Columbia University
 * COMS W4118 Fall 2014
 * Homework 6 - Geo Tagged File System
 */
#include <sys/syscall.h>

struct gps_location {
	double latitude;
	double longitude;
	float  accuracy; 
};

static inline int get_gps_location(const char *pathname,
				   struct gps_location *loc)
{
	return syscall(__NR_get_gps_location, pathname, loc);
}

#endif

