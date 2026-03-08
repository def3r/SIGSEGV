#include <kernel/terminal.h>
#include <kernel/vga.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFSIZE 100
#define DEC(x, n) (x <= n) ? 0 : x - n

typedef struct CursorPos {
  size_t row;
  size_t col;
} CursorPos;

typedef struct WriteScreen {
  bool dirty[VGA_HEIGHT];
} WriteScreen;

static uint16_t buffer[BUFSIZE][VGA_WIDTH];
static CursorPos cursor = {0, 0};
static size_t line_range_min = 0;
static size_t line_range_max = 0;
static size_t total_lines = 0;
static bool state_lock = false;
static bool disable_scroll = false;
static WriteScreen screen;
static uint8_t vga_color;

void term_set_cursor_block() {
  vga_set_cursor_block();
}
void term_set_cursor_underline() {
  vga_set_cursor_underline();
}

void term_scroll() {}

bool term_must_scroll() {
  return (cursor.row < line_range_min || cursor.row > line_range_max);
}

size_t term_get_cur_memline_len() {
  size_t n = 0;
  uint16_t* curline = buffer[cursor.row];
  for (; n < VGA_WIDTH; n++) {
    if ((curline[n] & 0xFF) == '\0') {
      break;
    }
  }
  return n;
}

void term_inschar(char c) {
  unsigned char uc = c;

  if (uc == '\r') {
    cursor.col = 0;
  } else if (uc == '\n') {
    cursor.col = 0;
    cursor.row++;
    total_lines++;
    screen.dirty[cursor.row - line_range_min] = true;
  } else {
    size_t curline_len = term_get_cur_memline_len();
    if (curline_len != cursor.col + 1) {
      uint16_t* curline = buffer[cursor.row];
      if (curline_len + 1 < VGA_WIDTH) {
        for (size_t y = curline_len; y > cursor.col; y--) {
          curline[y] = curline[y - 1];
        }
      } else {
        // TODO: Shift all lines following by 1
      }
    }
    buffer[cursor.row][cursor.col] = vga_entry(uc, vga_color);
    cursor.col++;
    screen.dirty[cursor.row - line_range_min] = true;
  }
  if (cursor.col == VGA_WIDTH) {
    cursor.col = 0;
    cursor.row++;
    screen.dirty[cursor.row - line_range_min] = true;
    if (!state_lock) {
      total_lines++;
    }
  }

  // Actually scroll through the memory
  // TODO: buf sync
  // if (!disable_scroll && term_must_scroll()) {
  //   term_scroll();
  // }
}

void term_putchar(char c) {
  unsigned char uc = c;

  if (uc == '\r') {
    cursor.col = 0;
  } else if (uc == '\n') {
    cursor.col = 0;
    cursor.row++;
    total_lines++;
    screen.dirty[cursor.row - line_range_min] = true;
  } else {
    buffer[cursor.row][cursor.col] = vga_entry(uc, vga_color);
    cursor.col++;
    screen.dirty[cursor.row - line_range_min] = true;
  }
  if (cursor.col == VGA_WIDTH) {
    cursor.col = 0;
    cursor.row++;
    screen.dirty[cursor.row - line_range_min] = true;
    if (!state_lock) {
      total_lines++;
    }
  }

  // Actually scroll through the memory
  // TODO: buf sync
  // if (!disable_scroll && term_must_scroll()) {
  //   term_scroll();
  // }
}

void term_flush() {
  for (size_t row = 0; row < VGA_HEIGHT; row++) {
    if (!screen.dirty[row]) {
      continue;
    }
    vga_mem_cpy(row, 0, (uint16_t*)buffer[row], VGA_WIDTH);
    screen.dirty[row] = false;
  }
  vga_set_cursor_pos(cursor.row, cursor.col);
}

void term_write(const char* data, size_t size) {
  screen.dirty[cursor.row - line_range_min] = true;
  for (size_t i = 0; i < size; i++) {
    term_putchar(data[i]);
  }
  term_flush();
}

typedef enum TermState { NORMAL = 0, INSERT = 1 } TermState;
typedef enum Direction { FWD = 1, BWD = -1 } Direction;

static TermState mode = NORMAL;
static vga_pos stlpos;
static enum vga_color stl_bgcolor[2] = {VGA_COLOR_BROWN, VGA_COLOR_GREEN};
static enum vga_color stl_fgcolor[2] = {VGA_COLOR_WHITE, VGA_COLOR_WHITE};

void term_memset(uint16_t* dest, uint16_t uc, size_t size) {
  for (size_t i = 0; i < size; i++) {
    dest[i] = uc;
  }
}

static void term_draw_statusline() {
  uint8_t save_color = vga_color;
  vga_color = vga_entry_color(stl_fgcolor[mode], stl_bgcolor[mode]);
  term_memset(buffer[VGA_HEIGHT - 1], vga_entry(' ', vga_color), VGA_WIDTH);
  CursorPos save_cursor = cursor;
  cursor.row = VGA_HEIGHT - 1;
  cursor.col = 0;
  // clang-format off
  printf(" [ %s ] (%d, %d):%d",
      mode == NORMAL ? "NORMAL" : "INSERT",
      save_cursor.row, save_cursor.col,
      total_lines
  );
  // clang-format on
  vga_color = save_color;
  cursor = save_cursor;
  screen.dirty[VGA_HEIGHT - 1] = true;
}

static void term_scroll_cb() {
  size_t vga_cur_range[2] = {0, 0};
  // vga_get_cur_memline_range(vga_cur_range);
  // vga_swap_memline(stlpos.row, vga_cur_range[1] + 1);
  stlpos.row = vga_cur_range[1] + 1, stlpos.col = 0;
}

static bool term_not_aiswspace(const char c) {
  return !aiswspace(c);
}

static bool term_under_aiaplhanum(const char c) {
  return aisalphanum(c) || c == '_';
}

static void term_skip_word(Direction dir, bool (*cmp)(const char)) {
  // TODO: skip if reached end of line for now
  if ((cursor.col == VGA_WIDTH - 1 && dir == FWD) ||
      (cursor.col == 0 && dir == BWD)) {
    return;
  }

  uint16_t* curline = buffer[cursor.row];
  size_t y = cursor.col;
  y = y + dir;
  while (y < VGA_WIDTH && aiswspace(curline[y] & 0xFF)) {
    y = y + dir;
  }
  while (y < VGA_WIDTH && cmp(curline[y] & 0xFF)) {
    y = y + dir;
  }
  y += -1 * dir * (y != cursor.col + dir);
  cursor.col = y;
}

static void term_skip_short_word(Direction dir) {
  term_skip_word(dir, term_under_aiaplhanum);
}

static void term_skip_long_word(Direction dir) {
  term_skip_word(dir, term_not_aiswspace);
}

static void term_skip_to_last_char() {
  cursor.col = term_get_cur_memline_len();
}

void term_cursor_left(size_t n) {
  cursor.col = DEC(cursor.col, n);
}

void term_cursor_right(size_t n) {
  cursor.col = MIN(cursor.col + n, VGA_WIDTH - 1);
}

void term_cursor_up(size_t n) {
  cursor.row = DEC(cursor.row, n);
}

void term_cursor_down(size_t n) {
  cursor.row = MIN(cursor.row + n, total_lines);
}

static void term_skip_to_first_char() {
  uint16_t* curline = buffer[cursor.row];
  size_t y = 0;
  for (; y < MIN(VGA_WIDTH, term_get_cur_memline_len()) &&
         aiswspace(curline[y] & 0xFF);
       y++)
    ;
  cursor.col = y;
}

static void term_del_char(Direction dir) {
  if (cursor.col == 0 && dir == BWD) {
    return;
  }
  uint16_t* curline = buffer[cursor.row];
  for (size_t y = cursor.col + (dir == FWD); y < VGA_WIDTH; y++) {
    curline[y - 1] = curline[y];
  }
  // TODO: This should technically bring next line char to cur line but for
  // now we just null the last char to avoid duplication
  curline[VGA_WIDTH - 1] = (curline[VGA_WIDTH - 1] & 0xFF00) | '\0';
  if (dir == BWD) {
    term_cursor_left(1);
  }
  screen.dirty[cursor.row] = true;
}

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
      mode = INSERT;
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
      mode = NORMAL;
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
  if (mode == NORMAL) {
    handle_normal_mode(in);
  } else if (mode == INSERT) {
    handle_insert_mode(in);
  }
  term_update();
}

void term_init() {
  vga_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
  for (size_t row = 0; row < BUFSIZE; row++) {
    for (size_t col = 0; col < VGA_WIDTH; col++) {
      buffer[row][col] = vga_entry('\0', vga_color);
    }
  }

  for (uint32_t i = 0; i < VGA_HEIGHT; i++) {
    screen.dirty[i] = true;
  }
  // stlpos.row = vga_screen_height() - 1, stlpos.col = 0;
  term_update();
  // vga_set_max_lines(stlpos.row - 1);
  // vga_set_scroll_cb(term_scroll_cb);
}

void term_update() {
  term_draw_statusline();
  term_flush();
}
