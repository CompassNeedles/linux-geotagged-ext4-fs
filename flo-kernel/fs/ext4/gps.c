/*
 * Interface between ext4 and gps data structs.
 *
 */
#ifdef CONFIG_GPSFS
#include "gps.h"
#include "ext4.h"

int set_gps_location_ext4(struct inode *inode)
{
    int error = 0;
    struct gps_location loc;
    struct ext4_iloc iloc;
    struct ext4_inode *raw_inode;

    struct timespec t;
    struct timespec c;
    __u32 time_info;
    long nanoseconds;
    __kernel_time_t seconds;

    error = ext4_get_inode_loc(inode, &iloc);
    if (error)
        goto skip_set;

    raw_inode = ext4_raw_inode(&iloc);
    if (!raw_inode) {
        error = -ENODEV;
        goto skip_set;
    }

    /* update ext4_inode gps data and time _stamp_ */
    get_current_gps_location(&loc);
    memcpy(&raw_inode->i_latitude, &loc.latitude,
        sizeof(raw_inode->i_latitude));
    memcpy(&raw_inode->i_longitude, &loc.longitude,
        sizeof(raw_inode->i_longitude));
    memcpy(&raw_inode->i_accuracy, &loc.accuracy,
        sizeof(raw_inode->i_accuracy));

    /* time _stamp_ */
    get_last_gps_update_time(&t); /* is locking */
    c = current_kernel_time();

    /* It is easy to interpret values in userspace
     * based on whether the gpsd
     * daemon is still running, its periodicty AND
     * the modification time of a
     * file. Using all this information it is possible
     * to determine the coord
     * units of age. Just check:
    printk("SETGPSL, time diff - sec: %llu, nsec: %lu.\n",
        (long long unsigned)(c.tv_sec - t.tv_sec),
        (long unsigned)(c.tv_nsec - t.tv_sec));
     */

    if (c.tv_sec == t.tv_sec) {
        nanoseconds = c.tv_nsec - t.tv_nsec;
        memcpy(&time_info, &nanoseconds, sizeof(time_info));
    } else {
        seconds = c.tv_sec - t.tv_sec;

        if (seconds >= 0x7fffffff) /* since on err return < 0 */
            time_info = 0x7fffffff;
        else
            memcpy(&time_info, &seconds, sizeof(time_info));
    }
    raw_inode->i_coord_age = cpu_to_le32(time_info);

    /* after copying in userland-given format reinforce endianness */
    raw_inode->i_latitude = cpu_to_le64(raw_inode->i_latitude);
    raw_inode->i_longitude = cpu_to_le64(raw_inode->i_longitude);
    raw_inode->i_accuracy = cpu_to_le32(raw_inode->i_accuracy);

    /* The common practice(s) */
    brelse(iloc.bh);
    ext4_set_inode_flags(inode);
    unlock_new_inode(inode);
    return error;
skip_set:
    brelse(iloc.bh);
    iget_failed(inode);
    return error;
}

int gps_info_ext4(struct inode *inode, struct gps_location *loc,
    unsigned long *age)
{
    int error = 0;
    struct ext4_iloc iloc;
    struct ext4_inode *raw_inode;

    if (!test_opt(inode->i_sb, GPS_AWARE_INODE))
        return -ENODEV;

    error = ext4_get_inode_loc(inode, &iloc);
    if (error)
        goto skip_info;

    /* This does pointer and offset math. Either something is messed up
     * somewhere else or it must work. */
    raw_inode = ext4_raw_inode(&iloc);

    /* export ext4_inode gps data. */
    if (raw_inode) {
        long long tmp_lat = le64_to_cpu(raw_inode->i_latitude);
        long long tmp_lng = le64_to_cpu(raw_inode->i_longitude);
        long tmp_acc = le32_to_cpu(raw_inode->i_accuracy);
        unsigned long age_temp = le32_to_cpu(raw_inode->i_coord_age);
        
        memcpy(&loc->longitude, &tmp_lng, sizeof(loc->longitude));
        imemcpy(&loc->accuracy, &tmp_acc, sizeof(loc->accuracy));
        memcpy(age, &age_temp, sizeof(*age));

        /* again, The common practices */
        brelse(iloc.bh);
        ext4_set_inode_flags(inode);
        unlock_new_inode(inode);
        return error;

    } else
        error = -ENODEV;

skip_info:
    brelse(iloc.bh);
    iget_failed(inode);
    return error;
}

int get_gps_location_ext4(struct inode *inode, struct gps_location *loc)
{
    int error = 0;
    struct ext4_iloc iloc;
    struct ext4_inode *raw_inode;

    error = ext4_get_inode_loc(inode, &iloc);
    if (error)
        goto skip_get;

    raw_inode = ext4_raw_inode(&iloc);

    /* export ext4_inode gps data. Lock? */
    if (raw_inode) {
        long long tmp_lat = le64_to_cpu(raw_inode->i_latitude);
        long long tmp_lng = le64_to_cpu(raw_inode->i_longitude);
        long tmp_acc = le32_to_cpu(raw_inode->i_accuracy);

        memcpy(&loc->latitude, &tmp_lat, sizeof(loc->latitude));
        memcpy(&loc->longitude, &tmp_lng, sizeof(loc->longitude));
        memcpy(&loc->accuracy, &tmp_acc, sizeof(loc->accuracy));

        /* again, The common practices */
        brelse(iloc.bh);
        ext4_set_inode_flags(inode);
        unlock_new_inode(inode);
        return error;

    } else
        error = -ENODEV;

skip_get:
    brelse(iloc.bh);
    iget_failed(inode);
    return error;
}

#endif
