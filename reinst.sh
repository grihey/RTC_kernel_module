#!/bin/bash
#cleanup module
rmmod -f rtc_romanov.ko

#install module
insmod rtc_romanov.ko

#Set behavior model
if [ "$1" == "normal" ]; 
then
  echo "s 150" > /proc/rtcromanov
fi

if [ "$1" == "random" ]; then
  echo "r 1" > /proc/rtcromanov
  echo "b 10000" > /proc/rtcromanov
fi
echo ""
echo "After init from sysclock, unix secs in module is \n"
cat /proc/rtcromanov

echo ""
echo "After set date to 11/11/99 17:30:00 time in module is \n"
hwclock -f /dev/rtc1 --set --date="11/11/99 17:30:00"
hwclock -f /dev/rtc1

