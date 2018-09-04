#!/bin/sh

if [ -a /dev/sda1 ];
then
    mount /dev/sda1 /media/card
fi

./mys-sam9x25_factory_test -a

rmmod g_mass_storage

if [ -a /dev/sda1 ];
then
    umount /media/card
fi
