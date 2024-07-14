CONFIG_MODULE_SIG=n
MAKE := make
ifneq ($(KERNELRELEASE),)
	obj-m := global.o
else
	KERNELDIR ?= /home/hl/kernel/linux-5.15.1
	PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif

clean:
	rm -rf *.o *.ko *.order *.symvers *.mod.c *.mod .*

