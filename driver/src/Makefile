export ARCH = arm
export CROSS_COMPILE = /opt/toolchains/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-
OUT = $(shell pwd)/../bin

KERNELVERSION=1587f77
KDIR := ../../buildroot-2013.11/output/build/linux-$(KERNELVERSION)/
MODDIR = ../../rootfs_files/lib/modules/$(KERNELVERSION)/extra
MKDIR = mkdir -p
CP = cp

.PHONY: all clean install

all:
	$(MAKE) -C $(KDIR) M=$$PWD

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

install: all $(MODDIR)
	$(CP) *.ko $(MODDIR)

$(OUT):
	$(MKDIR) $(OUT)

$(MODDIR):
	$(MKDIR) $(MODDIR)
