#!/bin/bash
rmmod -f module.ko
insmod module.ko
dmesg | tail -n 10
