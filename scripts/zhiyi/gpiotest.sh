#!/bin/sh

UDELAY=100000
#GPIOS=`ls -d /sys/class/gpio/pioB*`
GPIO_GROUP_1="/sys/class/gpio/pioB14 /sys/class/gpio/pioB16 /sys/class/gpio/pioB18 /sys/class/gpio/pioB20";
GPIO_GROUP_2="/sys/class/gpio/pioB15 /sys/class/gpio/pioB17 /sys/class/gpio/pioB19 /sys/class/gpio/pioB21";

#
# $1: The gpio group array
# $2: "out" for initialize this group as output
#     "in" for set as input
#
init_gpio()
{
	for gpio in $1;
	do
		echo $2 > $gpio/direction;
	done
}

#
# $1: the gpio group array
# $2: "0" for low and "1" for high
# $3: The index of gpio group set as an reverse value
#     if $2="0", this index set as "1"
#     if $2="1", this index set as "0"
#
set_gpio()
{
	gpio_idx=0;
	reverse_value=0;

	if [ $2 -eq 0 ]; then
		reverse_value=1;
	fi
	
	for gpio in $1;
	do
		if [ $gpio_idx -eq $3 ]; then
			echo $reverse_value > $gpio/value
		else
			echo $2 > $gpio/value
		fi
		let "gpio_idx = $gpio_idx + 1"
	done
}

#
# $1: the gpio group array
# $2: except value from gpio
# $3: the index of gpio group should get a reverse value of except vale
#
get_gpio()
{
	gpio_idx=0;
	reverse_value=0;

	if [ $2 -eq 0 ]; then
		reverse_value=1;
	fi

	for gpio in $1;
	do
		gpio_value=`cat $gpio/value`
		if [ $gpio_idx -eq $3 ]; then
			if [ $gpio_value -ne $reverse_value ]; then
				echo "Test on $gpio failed, expect $reverse_vale but got $gpio_value";
				exit -1;
			fi
		else
			if [ $gpio_value -ne $2 ]; then
				echo "Test on $gpio failed, expect $2 but got $gpio_value";
				exit -1
			fi
		fi
		let "gpio_idx = $gpio_idx + 1"
	done
}

test_gpio()
{
	loop_idx=0;
	
	# GPIO_GROUP_1 as input and GPIO_GROUP_2 as output
	init_gpio "$GPIO_GROUP_1" "in";
	init_gpio "$GPIO_GROUP_2" "out";

	while [ $loop_idx -lt 4 ]; do
		set_gpio "$GPIO_GROUP_2" 0 $loop_idx;
		get_gpio "$GPIO_GROUP_1" 0 $loop_idx;
		let "loop_idx = $loop_idx + 1"
	done

	# GPIO_GROUP_1 as output and GPIO_GROUP_2 as input
	init_gpio "$GPIO_GROUP_2" "in";
	init_gpio "$GPIO_GROUP_1" "out";

	let "loop_idx=0";
	while [ $loop_idx -lt 4 ]; do
		set_gpio "$GPIO_GROUP_1" 0 $loop_idx;
		get_gpio "$GPIO_GROUP_2" 0 $loop_idx;
		let "loop_idx = $loop_idx + 1"
	done
}

test_gpio

exit 0
