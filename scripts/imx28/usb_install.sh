#!/bin/sh

if [ $1 = "uninstall" ]; then
	if [ -d /media/sda1 ]; then
		umount /media/sda1
		rmdir /media/sda1
	fi

	rmmod g_mass_storage.ko
	
	exit 0
fi

mkfs.ext2 /dev/ram0

insmod g_mass_storage.ko stall=0 removable=1 file=/dev/ram0
if [ $? -ne 0 ];then
	echo "Cannot insmod g_mass_storage.ko"
    exit 1;   
fi

mkdir -p /media/sda1
mount -o rw,sync /dev/ram0 /media/sda1
if [ $? -ne 0 ];then
    echo "Cannot mount /dev/ram0 to /media/sda1"
	exit 1;
fi

exit 0
