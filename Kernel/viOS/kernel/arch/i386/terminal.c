#include <kernel/terminal.h>
#include <kernel/vga.h>
#include <stdint.h>
#include <stdio.h>

typedef enum TermState { NORMAL = 0, INSERT = 1 } TermState;

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
  }
}

static void handle_insert_mode(uint8_t in) {
  switch (in) {
    case 27:  // Esc
      mode = NORMAL;
      vga_set_cursor_block();
      term_update();
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
