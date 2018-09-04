#!/bin/sh

if [ $? -ne 0 ];then
	exit 1;
fi

insmod ./g_mass_storage.ko stall=0 removable=1 file=/dev/ram1
if [ $? -ne 0 ];then   
    exit 1;        
fi

