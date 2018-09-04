#!/bin/sh

sleep 5

cd /media

boot_img=`find . -name boot.img | awk 'NR==1 {print}'`
filesystem_img=`find . -name fsimage20.ubi | awk 'NR==1 {print}'`

check_file()
{
	if [ -z $boot_img ]; then
		echo "Not found the boot.img"
		exit 1
	fi

	if [ -z $filesystem_img ]; then
		echo "Not found the fsimage20.ubi"
		exit 1
	fi
}

update_boot()
{
	mtd_debug erase /dev/mtd0 0 8388608
	if [ $? -ne 0 ]; then
		echo "Failed to erase /dev/mtd0"
		exit 1
	fi

	mtd_debug write /dev/mtd0 0 8388608 $boot_img
	if [ $? -ne 0 ]; then
		exit 1
	fi
}

update_fs()
{
	flash_erase /dev/mtd1 0 0
	if [ $? -ne 0 ]; then
		echo "Failed to erase /dev/mtd1"
		exit 1
	fi

	ubiformat /dev/mtd1 -f $filesystem_img
	if [ $? -ne 0 ]; then
		echo "Failed to format /dev/mtd1 with fsimage20.ubi"
		exit 1
	fi
}

check_file

update_boot
update_fs

while true; do
	echo 0 > /sys/class/gpio/gpio113/value
	sleep 1
	echo 1 > /sys/class/gpio/gpio113/value
done

exit 0
