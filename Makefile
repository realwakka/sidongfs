obj-m := sidongfs.o

sidongfs-objs := inode.o dir.o

KERNEL_DIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
mkfs:
	cc mkfs.c -o mkfs.sidongfs
