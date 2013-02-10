obj-m := minos.o

CMD	:= $(MAKE)
KDIR := /lib/modules/$(shell uname -r)/build

all:
	$(CMD) -C $(KDIR) M=$(PWD) modules

clean:
	$(CMD) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean