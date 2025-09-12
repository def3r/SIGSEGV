KDIR := /lib/modules/`uname -r`/build

PWD := `pwd`

USER_PROGS := config filler reader
USER_CFLAGS := -Wall -Wextra -O2

all: module users

module:
	$(MAKE) -C $(KDIR) M=$(PWD)

users: $(USER_PROGS)

$(USER_PROGS): %: %.c
	$(CC) $(USER_CFLAGS) -o $@ $<

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -rf $(USER_PROGS)
