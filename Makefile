CROSS_COMPILE=aarch64-linux-gnu-
CC=$(CROSS_COMPILE)gcc
CFLAGS= -g -Wall
obj-m += motor_driver.o
KDIR=/home/stanley/linux

.PHONY: all clean

all: modules monitor

modules:
	make -C $(KDIR) ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) M=$(shell pwd) modules

monitor: monitor.c
	$(CC) $(CFLAGS) -o monitor monitor.c
clean:
	make -C $(KDIR) ARCH=arm64 CROSS_COMPILE=$(CROSS_COMPILE) M=$(shell pwd) clean
	rm -f monitor
