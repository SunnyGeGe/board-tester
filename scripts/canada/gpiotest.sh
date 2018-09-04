#!/bin/sh

UDELAY=100000
#GPIOS=`ls -d /sys/class/gpio/pioB*`
# GPIO_GROUP_1 default as input
GPIO_GROUP_1="/sys/class/gpio/gpio56 /sys/class/gpio/gpio55 /sys/class/gpio/gpio57 /sys/class/gpio/gpio40 /sys/class/gpio/gpio33 /sys/class/gpio/gpio38 /sys/class/gpio/gpio97";
# GPIO_GROUP_2 default as output
GPIO_GROUP_2="/sys/class/gpio/gpio53 /sys/class/gpio/gpio54 /sys/class/gpio/gpio58 /sys/class/gpio/gpio39 /sys/class/gpio/gpio32 /sys/class/gpio/gpio37 /sys/class/gpio/gpio59";

#
# $1: The gpio group array
# $2: "out" for initialize this group as output
#     "in" for set as input
#
init_gpio()
{
	for gpio in $1;
	do
		if [ ! -d $gpio ]; then
			echo ${gpio##*gpio} > ${gpio%gpio*}export
		fi
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

	# echo -n "set:  " # Debug
	for gpio in $1;
	do
		if [ $gpio_idx -eq $3 ]; then
			echo $reverse_value > $gpio/value
			# echo -n "$reverse_value " # Debug 
		else
			echo $2 > $gpio/value
			# echo -n "$2 " # Debug 
		fi
		let "gpio_idx = $gpio_idx + 1"
	done
	# echo "" # Debug 
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

	# echo -n "got:  " # Debug 
	for gpio in $1;
	do
		gpio_value=`cat $gpio/value`
		# echo -n "$gpio_value " # Debug
		if [ $gpio_idx -eq $3 ]; then
			if [ $gpio_value -ne $reverse_value ]; then
				echo "Test on $gpio failed, expect $reverse_value but got $gpio_value";
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
	# echo "" # Debug
	# echo "" # Debug 
}

get_reverse_gpio()
{
	gpio_idx=0;
	reverse_value=0;

	if [ $2 -eq 0 ]; then
		reverse_value=1;
	fi

	# echo -n "got:  " # Debug 
	for gpio in $1;
	do
		gpio_value=`cat $gpio/value`
		# echo -n "$gpio_value " # Debug
		if [ $gpio_idx -eq $3 ]; then
			if [ "$gpio" = "/sys/class/gpio/gpio97" ] || [ "$gpio" = "/sys/class/gpio/gpio38" ]; then
				if [ $gpio_value -ne $2 ]; then
					echo "Test on $gpio failed, expect $reverse_value but got $gpio_value";
					exit -1;
				fi
			else
				if [ $gpio_value -ne $reverse_value ]; then
					echo "Test on $gpio failed, expect $reverse_value but got $gpio_value";
					exit -1;
				fi
			fi
		else
			if [ "$gpio" = "/sys/class/gpio/gpio97" ] || [ "$gpio" = "/sys/class/gpio/gpio38" ]; then
				if [ $gpio_value -ne $reverse_value ]; then
					echo "Test on $gpio failed, expect $2 but got $gpio_value cc";
					exit -1
				fi
			else
				if [ $gpio_value -ne $2 ]; then
					echo "Test on $gpio failed, expect $2 but got $gpio_value dd";
					exit -1
				fi
			fi
		fi
		let "gpio_idx = $gpio_idx + 1"
	done
	# echo "" # Debug
	# echo "" # Debug 
}

# GPIO_REVERSE_GROUP_1 default as input
GPIO_REVERSE_GROUP_1="/sys/class/gpio/gpio97 /sys/class/gpio/gpio119 /sys/class/gpio/gpio38";
# GPIO_REVERSE_GROUP_2 default as ouput
GPIO_REVERSE_GROUP_2="/sys/class/gpio/gpio148 /sys/class/gpio/gpio37 /sys/class/gpio/gpio118";

test_gpio()
{
	loop_idx=0;
	loop_count=0;
	reverse_count=0;

	#Get the count of per GROUP
	for gpio in $GPIO_GROUP_1;
	do
		let "loop_count = $loop_count + 1"
	done

	# Set the multiplexers
	echo 99 > /sys/class/gpio/export
	echo out > /sys/class/gpio/gpio99/direction
	echo 1 > /sys/class/gpio/gpio99/value
	
	# GPIO_GROUP_1 as input and GPIO_GROUP_2 as output
	init_gpio "$GPIO_GROUP_1" "in";
	init_gpio "$GPIO_GROUP_2" "out";

	while [ $loop_idx -lt $loop_count ]; do
		set_gpio "$GPIO_GROUP_2" 0 $loop_idx;
		get_gpio "$GPIO_GROUP_1" 0 $loop_idx;
		let "loop_idx = $loop_idx + 1"
	done

	# GPIO_GROUP_1 as output and GPIO_GROUP_2 as input
#	init_gpio "$GPIO_GROUP_2" "in";
#	init_gpio "$GPIO_GROUP_1" "out";

#	let "loop_idx=0";
#	while [ $loop_idx -lt 4 ]; do
#		set_gpio "$GPIO_GROUP_1" 0 $loop_idx;
#		get_gpio "$GPIO_GROUP_2" 0 $loop_idx;
#		let "loop_idx = $loop_idx + 1"
#	done

	#Get the count of per GROUP
	for gpio in $GPIO_REVERSE_GROUP_1;
	do
		let "reverse_count = $reverse_count + 1"
	done

	# Set the multiplexers
	echo 0 > /sys/class/gpio/gpio99/value
	
	# Reverse GPIO test
	init_gpio "$GPIO_REVERSE_GROUP_1" "in";
	init_gpio "$GPIO_REVERSE_GROUP_2" "out";

	let "loop_idx=0";
	while [ $loop_idx -lt $reverse_count ]; do
		set_gpio "$GPIO_REVERSE_GROUP_2" 0 $loop_idx;
		get_reverse_gpio "$GPIO_REVERSE_GROUP_1" 0 $loop_idx;
		let "loop_idx = $loop_idx + 1"
	done

}

test_gpio

exit 0
