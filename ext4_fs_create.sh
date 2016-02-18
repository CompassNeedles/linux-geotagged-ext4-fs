dd if=/dev/zero of=geotagged.fs bs=1M count=2
sudo losetup /dev/loop0 geotagged.fs
sudo ./userspace/e2fsprogs/misc/mke2fs -I 256 -t ext4 -L w4118.GTF /dev/loop0
sudo losetup -d /dev/loop0
