KDIR ?= /lib/modules/`uname -r`/build

default:
	$(MAKE) ARCH=arm CROSS_COMPILE=arm-none-eabi- -C $(KDIR) M=$$PWD

clean:
	rm -rf *.mod *.mod.c *.ko *.o .*.cmd .tmp_versions Module.markers modules.order Module.symvers built-in.a 
