KERNELDIR = /usr/src/linux-headers-`uname -r` 
obj-m += module.o
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
