# https://linux-kernel-labs.github.io/refs/heads/master/labs/kernel_modules.html#compiling-kernel-modules

# This invokes make in the kernel src dir
KDIR := /lib/modules/`uname -r`/build

PWD := `pwd`

all:
	$(MAKE) -C $(KDIR) M=$(PWD)

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
