#include <kernel/terminal.h>

static void handle_normal_mode(uint8_t in) {
  uint8_t cursor_move = 0;
  Direction dir = 0;
  void (*handler)() = NULL;

  switch (in) {
    case 's':
      term_del_char(FWD);
      cursor_move = -1;
    case 'a':
      cursor_move++;
      term_cursor_right(cursor_move);
    case 'i':
      term_set_mode(INSERT);
      term_set_cursor_underline();
      term_update();
      break;

    case 'h':
      term_cursor_left(1);
      break;
    case 'j':
      term_cursor_down(1);
      break;
    case 'k':
      term_cursor_up(1);
      break;
    case 'l':
      term_cursor_right(1);
      break;

    // Line Editing
    case 'x':
      term_del_char(FWD);
      break;

    // Line motions
    case 'b':
      dir = BWD - 1;
    case 'e':
      dir++;
      term_skip_short_word(dir);
      break;

    case 'B':
      dir = BWD - 1;
    case 'E':
      dir++;
      term_skip_long_word(dir);
      break;

    case '$':
      term_skip_to_last_char();
      break;
    case '_':
      term_skip_to_first_char();
      break;

    default:
      if (handler) {
        handler();
      }
  }
}

static void handle_insert_mode(uint8_t in) {
  switch (in) {
    case 27:  // Esc
      term_set_mode(NORMAL);
      term_set_cursor_block();
      term_update();
      break;
    case '\b':
      term_del_char(BWD);
      break;
    case 13:  // Enter
      // TODO: Implement Enter
      in = '\n';
    default:
      term_inschar(in);
  }
}

void handle_input(uint8_t in) {
  TermMode mode = term_get_mode();
  if (mode == NORMAL) {
    handle_normal_mode(in);
  } else if (mode == INSERT) {
    handle_insert_mode(in);
  }
  term_update();
}
