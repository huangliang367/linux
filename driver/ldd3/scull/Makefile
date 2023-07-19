MAKE := make ARCH=arm64 
ifneq ($(KERNELRELEASE),)
	obj-m := scull.o
else
	KERNELDIR ?= /home/hl/code/rk356x_linux/kernel
	PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif

clean:
	rm -rf *.o *.ko *.order *.symvers *.mod.c
