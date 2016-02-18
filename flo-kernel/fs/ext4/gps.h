/*
  File: fs/ext4/gps.h

  Geo-tagged ext4 filesystem interface functions.

  (C) 2015, OSI, team12
*/
#ifdef CONFIG_GPSFS
#include <linux/fs.h>
#include <linux/gps.h>

int get_gps_location_ext4(struct inode *inode, struct gps_location *loc);
int set_gps_location_ext4(struct inode *inode);
int gps_info_ext4(struct inode *inode, struct gps_location *loc,
    unsigned long *age);

#endif
