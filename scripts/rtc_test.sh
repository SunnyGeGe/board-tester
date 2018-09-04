#!/bin/sh

hwclock -s

timea=`date +%s`

sleep 2

timeb=`date +%s`

#echo $timea, $timeb
ret=`expr $timeb - $timea`
if [ $ret != "2" ];
then
    exit 1;
else
    exit 0;
fi
