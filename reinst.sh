#!/bin/bash
rm /dev/rtc38
rmmod -f rtc_romanov.ko
insmod rtc_romanov.ko
mknod /dev/rtc38 c 250 0
