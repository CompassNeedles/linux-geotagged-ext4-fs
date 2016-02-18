#ifdef CONFIG_GPSFS
#define VERBOSE 0

#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/gps.h>
#include <linux/fs.h>

static DEFINE_RWLOCK(k_gps_lock);

static struct gps_location k_cur_loc = {
    .latitude = 0,
    .longitude = 0,
    .accuracy = 0
};

static struct timespec last_gps_update_time; /* Auto 0 */

int get_current_gps_location(struct gps_location *l)
{
    read_lock(&k_gps_lock);
    memcpy(l, &k_cur_loc, sizeof(*l));
    read_unlock(&k_gps_lock);
    return 0;
}

int get_last_gps_update_time(struct timespec *t)
{
    read_lock(&k_gps_lock);
    memcpy(t, &last_gps_update_time, sizeof(*t));
    read_unlock(&k_gps_lock);
    return 0;
}

SYSCALL_DEFINE1(set_gps_location, struct gps_location __user, *loc)
{
    struct gps_location k_loc;
    unsigned int num_bytes_uncopied;

    if (current_uid() != 0)
        return -EACCES;
    if (loc == NULL)
        return -EINVAL;

#if VERBOSE
    printk("\nSetting kernel GPS location struct with userspace data.\n");
#endif
    num_bytes_uncopied = copy_from_user(&k_loc, loc, sizeof(k_loc));
    if (num_bytes_uncopied) {
#if VERBOSE
        printk(
            "Kernel current location addr, size: %p:%u, %p:%u, %p:%u, %p:%u\n",
            &k_cur_loc, sizeof(k_cur_loc), &k_cur_loc.latitude,
            sizeof(k_cur_loc.latitude), &k_cur_loc.longitude,
            sizeof(k_cur_loc.longitude), &k_cur_loc.accuracy,
            sizeof(k_cur_loc.accuracy));
        printk("num bytes uncopied: %u\n", num_bytes_uncopied);
        printk("of: %u\n", sizeof(k_cur_loc));
#endif
        return -EFAULT;
    }
    write_lock(&k_gps_lock);
    memcpy(&k_cur_loc, &k_loc, sizeof(k_loc));
    last_gps_update_time = current_kernel_time();
    write_unlock(&k_gps_lock);
#if VERBOSE
    printk("\nKernel current location addr, size: %p:%u, %p:%u, %p:%u, %p:%u\n",
        &k_cur_loc, sizeof(k_cur_loc), &k_cur_loc.latitude,
        sizeof(k_cur_loc.latitude), &k_cur_loc.longitude,
        sizeof(k_cur_loc.longitude), &k_cur_loc.accuracy,
        sizeof(k_cur_loc.accuracy));
    printk("num bytes uncopied: %u\n", num_bytes_uncopied);
    printk("of: %u\n", sizeof(k_cur_loc));
#endif
    return 0;
}

SYSCALL_DEFINE2(get_gps_location, const char __user, *pathname,
    struct gps_location __user, *loc)
{
    struct gps_location loc_copy;
    long age;
    int error;
    unsigned int num_bytes_uncopied;

    if (pathname == NULL || loc == NULL)
        return -EINVAL;

    error = get_path_gps_aware_inode_data(pathname, &loc_copy, &age);
    if (error)
        return error;
    num_bytes_uncopied = copy_to_user(loc, &loc_copy, sizeof(*loc));
    printk("num bytes uncopied to user: %u\n", num_bytes_uncopied);
    printk("of: %u\n", sizeof(loc));
    if (num_bytes_uncopied)
        return -EFAULT;
    return age;
}

#endif
