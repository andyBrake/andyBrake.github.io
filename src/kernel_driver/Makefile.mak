ifneq ($(KERNELRELEASE),)

    obj-m := hello.o

else

    KERN_DIR ?= /usr/src/linux-headers-$(shell uname -r)/
    PWD := $(shell pwd)

default:
    $(MAKE) -C $(KERN_DIR) M=$(PWD) modules

endif


clean:
    rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions