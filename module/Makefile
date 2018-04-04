obj-m += rotary_volume.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
install:
	mkdir -p /lib/modules/$(shell uname -r)/kernel/drivers/input/keyboard/
	cp rotary_volume.ko /lib/modules/$(shell uname -r)/kernel/drivers/input/keyboard/
	chown root:root /lib/modules/$(shell uname -r)/kernel/drivers/input/keyboard/rotary_volume.ko
	depmod -a
