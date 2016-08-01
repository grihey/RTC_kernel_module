KERNELDIR = /usr/src/linux-headers-`uname -r` 
obj-m += rtc_romanov.o
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
