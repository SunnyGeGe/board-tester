#!/bin/sh

dev_exist=1
dev=$1

if [ ! -b /dev/$dev ]; then
	dev_exist=0
fi

if [ -d /media/$dev ]; then
	mountpoint /media/$dev > /dev/null
	if [ $? -eq 0 ]; then
		umount /media/$dev
	fi
else
	if [ $dev_exist -eq 1 ]; then
		mkdir -p /media/$dev
	fi
fi

if [ $dev_exist -eq 1 ]; then
	mount /dev/$dev /media/$dev
	if [ $? -ne 0 ]; then
		rm -r /media/$dev
	fi
else
	rm -r /media/$dev
fi



