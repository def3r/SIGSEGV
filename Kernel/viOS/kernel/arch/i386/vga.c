#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <system.h>

#include <kernel/vga.h>

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint32_t const VGA_MEMORYLOC = 0xB8000;
static uint16_t* const VGA_MEMORY = (uint16_t*)VGA_MEMORYLOC;

#define DEC(x, n) (x <= n) ? 0 : x - n

typedef enum vga_cursor_types { UNDERLINE, BLOCK } CursorTypes;

static uint16_t* vga_buffer;

static vga_pos vga_cursor;
static size_t vga_total_lines;
static size_t vga_max_lines;
static size_t vga_line_range_min;
static size_t vga_line_range_max;

static uint8_t vga_color;
static bool disable_scroll = false;
static bool state_lock = false;

static void (*vga_scroll_cb)();

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
  return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
  return (uint16_t)uc | (uint16_t)color << 8;
}

static void vga_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
  const size_t index = y * VGA_WIDTH + x;
  vga_buffer[index] = vga_entry(c, color);
}

// Fantasy
static void vga_scroll(int line) {
  int loop;
  char c;
  for (loop = line * (VGA_WIDTH * 2) + VGA_MEMORYLOC; loop < VGA_WIDTH * 2;
       loop++) {
    c = *(char*)loop;
    *(char*)(loop - (VGA_WIDTH * 2)) = c;
  }
}

// Actual Scroll
// http://www.osdever.net/FreeVGA/vga/crtcreg.htm#0C
// Accepts only 16 bits of offset (base is VGA_MEMORY)
static void vga_scroll2() {
  int8_t scroll = (vga_cursor.x < vga_line_range_min) ? -1 : 1;
  vga_line_range_min += scroll;
  vga_line_range_max += scroll;
  outb(0x3D4, 0x0D);
  outb(0x3D5, (vga_line_range_min * VGA_WIDTH & 0xFF));
  outb(0x3D4, 0x0C);
  outb(0x3D5, ((vga_line_range_min * VGA_WIDTH >> 8) & 0xFF));

  if (vga_scroll_cb) {
    vga_scroll_cb();
  }
}

bool vga_must_scroll() {
  return (vga_cursor.x < vga_line_range_min ||
          vga_cursor.x > vga_line_range_max);
}

static void vga_delete_last_line() {
  int x, *ptr;

  for (x = 0; x < VGA_WIDTH * 2; x++) {
    ptr = (int*)(VGA_MEMORYLOC + (VGA_WIDTH * 2) * (VGA_HEIGHT - 1) + x);
    *ptr = 0;
  }
}

// Ref: http://www.osdever.net/FreeVGA/vga/crtcreg.htm#0E
static void vga_update_cursor() {
  uint16_t pos = vga_cursor.x * VGA_WIDTH + vga_cursor.y;
  outb(0x3D4, 0x0F);
  outb(0x3D5, (pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, ((pos >> 8) & 0xFF));
}

// Ref: http://www.osdever.net/FreeVGA/vga/textcur.htm#shape
static void vga_set_cursor(CursorTypes c) {
  uint8_t line_start = 0x00;
  uint8_t line_end = 0x0F;
  if (c == UNDERLINE) {
    line_start = 0x0D;
  }

  outb(0x3D4, 0x0A);
  outb(0x3D5, line_start);
  outb(0x3D4, 0x0B);
  outb(0x3D5, line_end);
}
void vga_set_cursor_block() {
  vga_set_cursor(BLOCK);
}
void vga_set_cursor_underline() {
  vga_set_cursor(UNDERLINE);
}

void vga_init(void) {
  vga_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
  vga_buffer = VGA_MEMORY;
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = y * VGA_WIDTH + x;
      vga_buffer[index] = vga_entry(' ', vga_color);
    }
  }
  vga_max_lines = VGA_HEIGHT - 1;
  vga_line_range_min = 0;
  vga_line_range_max = vga_max_lines;
  vga_set_cursor_block();
}

void vga_putchar(char c) {
  unsigned char uc = c;

  if (uc == '\r') {
    vga_cursor.y = 0;
  } else if (uc == '\n') {
    vga_cursor.y = 0;
    vga_cursor.x++;
    vga_total_lines++;
  } else {
    vga_putentryat(uc, vga_color, vga_cursor.y, vga_cursor.x);
    vga_cursor.y++;
  }
  if (vga_cursor.y == VGA_WIDTH) {
    vga_cursor.y = 0;
    vga_cursor.x++;
    if (!state_lock) {
      vga_total_lines++;
    }
  }

  // Actually scroll through the memory
  if (!disable_scroll && vga_must_scroll()) {
    vga_scroll2();
  }
  vga_update_cursor();
}

void vga_write(const char* data, size_t size) {
  for (size_t i = 0; i < size; i++)
    vga_putchar(data[i]);
}

void vga_writestring(const char* data) {
  vga_write(data, strlen(data));
}

void vga_cursor_left(size_t n) {
  vga_cursor.y = DEC(vga_cursor.y, n);
  vga_update_cursor();
}

void vga_cursor_right(size_t n) {
  vga_cursor.y = MIN(vga_cursor.y + n, VGA_WIDTH - 1);
  vga_update_cursor();
}

void vga_cursor_up(size_t n) {
  vga_cursor.x = DEC(vga_cursor.x, n);
  if (!disable_scroll && vga_must_scroll()) {
    vga_scroll2();
  }
  vga_update_cursor();
}

void vga_cursor_down(size_t n) {
  vga_cursor.x = MIN(vga_cursor.x + n, vga_total_lines);
  if (!disable_scroll && vga_must_scroll()) {
    vga_scroll2();
  }
  vga_update_cursor();
}

uint8_t vga_screen_height() {
  return VGA_HEIGHT;
}

uint8_t vga_screen_width() {
  return VGA_WIDTH;
}

vga_pos vga_get_cursor_pos() {
  return (vga_pos){.x = vga_cursor.x, .y = vga_cursor.y};
}

void vga_set_cursor_pos(size_t x, size_t y, bool scroll2view) {
  vga_cursor.x = x, vga_cursor.y = y;
  vga_update_cursor();
  // TODO: Implement scroll2view
  if (scroll2view) {
    vga_scroll2();
  }
}

void vga_set_color_from(enum vga_color fg, enum vga_color bg) {
  vga_color = vga_entry_color(fg, bg);
}

void vga_set_color(uint8_t color) {
  vga_color = color;
}

uint8_t vga_get_color() {
  return vga_color;
}

void vga_memline_fill_with(size_t x, size_t y, char c) {
  vga_pos save_cursor = vga_cursor;
  vga_cursor.x = x, vga_cursor.y = y;
  state_lock = true;
  disable_scroll = true;
  for (uint8_t i = y; i < VGA_WIDTH; i++) {
    vga_putchar(c);
  }
  disable_scroll = false;
  state_lock = true;
  vga_cursor = save_cursor;
}

void vga_cur_memline_fill_with(char c) {
  vga_memline_fill_with(vga_cursor.x, 0, c);
}

void vga_set_max_lines(size_t n) {
  vga_max_lines = n;
  vga_line_range_max = n;
}

void vga_disable_scroll() {
  disable_scroll = true;
}

void vga_enable_scroll() {
  disable_scroll = false;
}

void vga_get_cur_memline_range(size_t res[2]) {
  res[0] = vga_line_range_min, res[1] = vga_line_range_max;
}

void vga_swap_memline(size_t row1, size_t row2) {
  size_t linewidth = VGA_WIDTH * 2;
  char bfr[linewidth];
  size_t pos1 = row1 * VGA_WIDTH;
  size_t pos2 = row2 * VGA_WIDTH;
  memcpy(bfr, VGA_MEMORY + pos1, linewidth);
  memcpy(VGA_MEMORY + pos1, VGA_MEMORY + pos2, linewidth);
  memcpy(VGA_MEMORY + pos2, bfr, linewidth);
}

void vga_set_scroll_cb(void (*cb)()) {
  vga_scroll_cb = cb;
}

size_t vga_get_total_lines() {
  return vga_total_lines;
}
