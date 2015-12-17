MODULE_NAME = gpio
KDIR = /home/bougar/drivers/rpi/kernel

TOOLCHAIN = /home/bougar/Compiladores/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-

TARGET = arm
PWD := $(shell pwd)
MODULE_UPLOAD = $(MODULE_NAME).ko

obj-m := $(MODULE_NAME).o

all:
	make -C $(KDIR) M=$(PWD) ARCH=$(TARGET) CROSS_COMPILE=$(TOOLCHAIN) modules

clean:
	make -C $(KDIR) M=$(PWD) ARCH=$(TARGET) CROSS_COMPILE=$(TOOLCHAIN) clean

install:
	@if [ -f $(MODULE_NAME).ko ]; \
	then \
		scp $(MODULE_NAME).ko pi@raspberry:/home/pi/drivers ; \
	else \
		echo "Run make first."; \
	fi 
