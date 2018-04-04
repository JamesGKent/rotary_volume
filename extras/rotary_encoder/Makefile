obj-m += rotary_encoder.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
install:
	cp rotary_encoder.ko /lib/modules/$(shell uname -r)/kernel/drivers/input/misc/
	chown root:root /lib/modules/$(shell uname -r)/kernel/drivers/input/misc/rotary_encoder.ko
	depmod -a
