#include <kernel/terminal.h>
#include <stdio.h>

void vios_init_proc() {
  term_init();

  printf("ViOS\n");
  printf("Init proc started!...\n");

  // TODO: Eliminate busy waiting
  while (1)
    ;
}
