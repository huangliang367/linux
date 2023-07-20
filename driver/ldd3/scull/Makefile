MAKE := make ARCH=arm64 
DEBUG = y

ifeq ($(DEBUG),y)
	DEBFLAGS = -O -g -DSCULL_DEBUG
else
	DEBFLAGS = -O2
endif

ccflags-y += $(DEBFLAGS)

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
