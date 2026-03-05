#include <kb.h>
#include <stdio.h>
#include <system.h>

#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/terminal.h>
#include <kernel/vga.h>

// Some Interesting Examples {{{

// Since the IDT is not made yet, and there is no gate for expception handling
// the system will reboot on an exception like this.
//
// This is the behaviour unitl we provide an ISR for interrupt handling,
void divBy0() {
  volatile int m = 0;  // needs to be volatile!
  int f = 5 / m;
  printf("res = %d",
         f);  // need to use f, otherwise compiler does unsed var elimination
  // This is so stupid, if:
  //    f = 5 / m; and m is not stated to be volatile, compiler would know its
  // an undefined behaviour at compile time and that would fire an interrupt 6
  // and not int 0!
}

// }}}

void kernel_main(void) {
  vga_init();
  gdt_init();
  idt_install();
  irq_install();
  timer_install();
  kb_install();
  __asm__ __volatile__("sti");  // enable interrupts

  // divBy0();
  printf("Interrupts enabled. Waiting...\n");
  printf("ViOS\n");

  term_init();

  // TODO: Eliminate busy waiting
  while (1) {
  }
}

// vim: foldmethod=marker
