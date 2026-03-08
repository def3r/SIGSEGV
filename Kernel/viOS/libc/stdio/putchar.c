#include <stdio.h>

#if defined(__is_libk)
#include <kernel/terminal.h>
#include <kernel/vga.h>
#endif

int putchar(int ic) {
#if defined(__is_libk)
  char c = (char)ic;
  // vga_write(&c, sizeof(c));
  // TODO: What happens if we use putchar when term is not init?
  // Need to check what is done in that case.. kprintf? kputchar?
  term_write(&c, sizeof(c));
#else
  // TODO: Implement stdio and the write system call.
#endif
  return ic;
}
