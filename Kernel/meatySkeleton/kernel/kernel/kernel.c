#include <stdio.h>

#include <kernel/gdt.h>
#include <kernel/tty.h>

void kernel_main(void) {
  gdt_init();
  terminal_initialize();

  printf("Hello, Kernel :)\n");
}
