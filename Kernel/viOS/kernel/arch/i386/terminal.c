#include <kernel/terminal.h>
#include <kernel/vga.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum TermState { NORMAL = 0, INSERT = 1 } TermState;
typedef enum Direction { FWD = 1, BWD = -1 } Direction;

static TermState mode = NORMAL;
static vga_pos stlpos;
static enum vga_color stl_bgcolor[2] = {VGA_COLOR_BROWN, VGA_COLOR_GREEN};
static enum vga_color stl_fgcolor[2] = {VGA_COLOR_WHITE, VGA_COLOR_WHITE};
static vga_pos save_cursor;

static void term_draw_statusline() {
  uint8_t save_color = vga_get_color();
  vga_set_color_from(stl_fgcolor[mode], stl_bgcolor[mode]);
  vga_cur_memline_fill_with(' ');
  vga_disable_scroll();
  // clang-format off
  printf(" [ %s ] (%d, %d):%d",
      mode == NORMAL ? "NORMAL" : "INSERT",
      save_cursor.x, save_cursor.y,
      vga_get_total_lines()
  );
  // clang-format on
  vga_enable_scroll();
  vga_set_color(save_color);
}

static void term_scroll_cb() {
  size_t vga_cur_range[2] = {0, 0};
  vga_get_cur_memline_range(vga_cur_range);
  vga_swap_memline(stlpos.x, vga_cur_range[1] + 1);
  stlpos.x = vga_cur_range[1] + 1, stlpos.y = 0;
}

static void term_skip_short_word(Direction dir) {
  save_cursor = vga_get_cursor_pos();
  // TODO: skip if reached end of line for now
  if ((save_cursor.y == VGA_WIDTH - 1 && dir == FWD) ||
      (save_cursor.y == 0 && dir == BWD)) {
    return;
  }

  uint16_t curline[VGA_WIDTH];
  vga_get_cur_memline((char*)curline);
  size_t y = save_cursor.y;
  y = y + dir;
  while (y < VGA_WIDTH && aiswspace(curline[y] & 0xFF)) {
    y = y + dir;
  }
  while (y < VGA_WIDTH && aisalphanum(curline[y] & 0xFF)) {
    y = y + dir;
  }
  y += -1 * dir * (y != save_cursor.y + dir);
  vga_set_cursor_pos(save_cursor.x, y, false);
}

static void term_skip_to_last_char() {
  save_cursor = vga_get_cursor_pos();
  vga_set_cursor_pos(save_cursor.x, vga_get_cur_memline_len(), false);
}

static void term_skip_to_first_char() {
  save_cursor = vga_get_cursor_pos();
  uint16_t curline[VGA_WIDTH];
  vga_get_cur_memline((char*)curline);
  size_t y = 0;
  for (; y < MIN(VGA_WIDTH, vga_get_cur_memline_len()) &&
         aiswspace(curline[y] & 0xFF);
       y++)
    ;
  vga_set_cursor_pos(save_cursor.x, y, false);
}

static void term_del_char(Direction dir) {
  save_cursor = vga_get_cursor_pos();
  if (save_cursor.y == 0 && dir == BWD) {
    return;
  }
  uint16_t res[VGA_WIDTH];
  vga_get_cur_memline((char*)res);
  for (size_t y = save_cursor.y + (dir == FWD); y < VGA_WIDTH; y++) {
    res[y - 1] = res[y];
  }
  // TODO: This should technically bring next line char to cur line but for
  // now we just null the last char to avoid duplication
  res[VGA_WIDTH - 1] = (res[VGA_WIDTH - 1] & 0xFF00) | '\0';
  vga_set_cur_memline((char*)res);
  if (dir == BWD) {
    vga_cursor_left(1);
  }
}

static void handle_normal_mode(uint8_t in) {
  switch (in) {
    case 'i':
      mode = INSERT;
      vga_set_cursor_underline();
      term_update();
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

    // Line motions
    case 'e':
      term_skip_short_word(FWD);
      break;
    case 'b':
      term_skip_short_word(BWD);
      break;
    case '$':
      term_skip_to_last_char();
      break;
    case '_':
      term_skip_to_first_char();
      break;

    // Line Editing
    case 'x':
      term_del_char(FWD);
      break;
  }
}

static void handle_insert_mode(uint8_t in) {
  switch (in) {
    case 27:  // Esc
      mode = NORMAL;
      vga_set_cursor_block();
      term_update();
      break;
    case '\b':
      term_del_char(BWD);
      break;
    case 13:  // Enter
      // TODO: Implement Enter
      in = '\n';
    default:
      vga_inschar(in);
  }
}

void handle_input(uint8_t in) {
  if (mode == NORMAL) {
    handle_normal_mode(in);
  } else if (mode == INSERT) {
    handle_insert_mode(in);
  }
  term_update();
}

void term_init() {
  stlpos.x = vga_screen_height() - 1, stlpos.y = 0;
  term_update();
  vga_set_max_lines(stlpos.x - 1);
  vga_set_scroll_cb(term_scroll_cb);
}

void term_update() {
  save_cursor = vga_get_cursor_pos();
  vga_set_cursor_pos(stlpos.x, stlpos.y, false);
  term_draw_statusline();
  vga_set_cursor_pos(save_cursor.x, save_cursor.y, false);
}
