#!/bin/sh

led_init()
{
	echo 56 > /sys/class/gpio/export
	echo out > /sys/class/gpio/pioB24/direction
}

led_on()
{
	echo 0 > /sys/class/gpio/pioB24/value
}

led_off()
{
	echo 1 > /sys/class/gpio/pioB24/value
}

#
# led_flash
#  $1 interval(us)
#  $2 times of flash, if $2 < 1, flash forever
#
led_flash()
{
	count=0;

	while [ $2 -lt 1 ] || [ $count -lt $2 ];
	do
		led_on
		usleep $1
		led_off
		usleep $1
		let "count = $count + 1";
	done
}

if [ "$1" = "init" ]; then
	led_init
elif [ "$1" = "on" ]; then
	led_on
elif [ "$1" = "off" ]; then
	led_off
elif [ "$1" = "flash" ]; then
	led_flash 200000 0
fi
	

