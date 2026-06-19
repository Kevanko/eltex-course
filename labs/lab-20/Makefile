obj-m += 01_hello.o 02_chardev.o 03_procfs.o 04_sysfs.o
KDIR ?= $(HOME)/study/eltex/labs/lab-17/linux
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
