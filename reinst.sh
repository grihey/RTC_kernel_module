#!/bin/bash
#cleanup module
rm /dev/rtc38
rmmod -f rtc_romanov.ko

#install module
insmod rtc_romanov.ko

#Major number sets by hands. You can see it in dmesg output after installing
#module
mknod /dev/rtc38 c 250 0

#Set behavior model
if [ "$1" == "normal" ]; 
then
  echo "s 150" > /proc/rtcromanov
fi

if [ "$1" == "random" ]; then
  echo "r 1" > /proc/rtcromanov
  echo "b 400" > /proc/rtcromanov
fi

cat /proc/rtcromanov

