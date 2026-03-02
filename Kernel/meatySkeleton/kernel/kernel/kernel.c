#include <stdio.h>

#include <kernel/gdt.h>
#include <kernel/tty.h>
#include "kernel/idt.h"

void kernel_main(void) {
  gdt_init();
  idt_install();
  terminal_initialize();

  printf("Welcome to Charlie Krik Linux!");
}
