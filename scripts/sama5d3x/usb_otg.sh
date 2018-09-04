#!/bin/sh

USB_OTG_DEV="sdb"

remove_mountpoint() {
	if [ -d /media/"$USB_OTG_DEV"1 ]; then
		mountpoint /media/"$USB_OTG_DEV"1 > /dev/null
		if [ $? -eq "0" ]; then
			umount /media/"$USB_OTG_DEV"1
		fi
		rmdir /media/"$USB_OTG_DEV"1
	fi
}

insmod /root/g_mass_storage.ko stall=1 file=/dev/ram1 removable=1

if [ $? -ne "0" ]; then
	echo "insmod g_mass_storage failed"
	remove_mountpoint
	exit 1;
fi

sleep 4

if [ ! -b /dev/$USB_OTG_DEV ]; then
	echo "Cannot found /dev/$USB_OTG_DEV"
	remove_mountpoint
	exit 1
fi

dd if=/dev/zero of=/dev/$USB_OTG_DEV bs=1024 count=1024
sfdisk /dev/$USB_OTG_DEV < /root/partition.data
if [ $? -ne "0" ]; then
	echo "Failed to partition /dev/$USB_OTG_DEV"
	remove_mountpoint
	exit 1
fi

sleep 2

mkfs.ext2 /dev/"$USB_OTG_DEV"1
if [ $? -ne "0" ]; then
	echo "Failed to format /dev/$USB_OTG_DEV"
	remove_mountpoint
	exit 1
fi

if [ ! -d /media/"$USB_OTG_DEV"1 ]; then
	mkdir /media/"$USB_OTG_DEV"1
fi

mount /dev/"$USB_OTG_DEV"1 /media/"$USB_OTG_DEV"1
if [ $? -ne "0" ]; then
	echo 'mount /dev/"$USB_OTG_DEV"1 failed'
	remove_mountpoint
	exit 1
fi
 
