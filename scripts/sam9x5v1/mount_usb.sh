#!/bin/sh

if grep $1 /proc/mounts > /dev/null; then
	exit 0;
fi

if [ ! -d /media/$1 ]; then
	mkdir /media/$1
fi

mount /dev/$1 /media/$1
if [ $? -ne 0 ]; then
	exit 1;
fi
