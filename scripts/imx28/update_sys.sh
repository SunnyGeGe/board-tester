#!/bin/sh

boot_img="/imx28_ivt_linux.sb"
rootfs_img="/rootfs.ubifs"

if [ ! -f $boot_img ] || [ ! -f $rootfs_img ]; then
	echo "Cannot found $boot_img/$rootfs_img, please check these files"
	exit 1
fi

flash_eraseall /dev/mtd0
if [ $? -ne 0 ]; then
	echo "Failed to erase /dev/mtd0"
	exit 1
fi

kobs-ng init $boot_img
flash_eraseall /dev/mtd0
if [ $? -ne 0 ]; then
	echo "Failed to update $boot_img"
	exit 1
fi

/ubiformat /dev/mtd1 -s 2048 -O 2048 -f $rootfs_img
if [ $? -ne 0 ]; then
	echo "Failed to update $rootfs_img"
	exit 1
fi
