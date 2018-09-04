#!/bin/sh

mkfs.fat /dev/ram0

modprobe atmel_usba_udc
if [ $? -ne 0 ];then
	exit 1;
fi

modprobe g_mass_storage stall=0 removable=1 file=/dev/ram0
if [ $? -ne 0 ];then   
    exit 1;        
fi

mkdir -p /dev/shm/usbshare

mount -t vfat -o rw,sync /dev/ram0 /dev/shm/usbshare
if [ $? -ne 0 ];then
    echo "failed!"
	exit 1;
fi

cp /dev/shm/bmp* /dev/shm/usbshare
