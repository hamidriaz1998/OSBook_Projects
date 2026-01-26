obj-m += time_elapsed.o
obj-m += simple.o
obj-m += hello.o
obj-m += jiffies_mod.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
