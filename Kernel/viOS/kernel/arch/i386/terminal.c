#include <kernel/vga.h>
#include <stdint.h>

typedef enum TermState { NORMAL, INSERT } TermState;

TermState mode = NORMAL;

static void handle_normal_mode(uint8_t in) {
  switch (in) {
    case 'i':
      mode = INSERT;
      vga_set_cursor_underline();
      break;
    case 'h':
      vga_cursor_left(1);
      break;
    case 'j':
      vga_cursor_down(1);
      break;
    case 'k':
      vga_cursor_up(1);
      break;
    case 'l':
      vga_cursor_right(1);
      break;
  }
}

static void handle_insert_mode(uint8_t in) {
  switch (in) {
    case 27:  // Esc
      mode = NORMAL;
      vga_set_cursor_block();
      break;
    default:
      vga_putchar(in);
  }
}

void handle_input(uint8_t in) {
  if (mode == NORMAL) {
    handle_normal_mode(in);
  } else if (mode == INSERT) {
    handle_insert_mode(in);
  }
}
