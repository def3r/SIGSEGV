#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system.h>

#include <kernel/vga.h>

#define DEC(x, n) (x <= n) ? 0 : x - n

typedef enum vga_cursor_types { UNDERLINE, BLOCK } CursorTypes;

static uint16_t* vga_buffer;

static vga_pos vga_cursor = {0, 0};
static size_t vga_total_lines;
static size_t vga_max_lines;
static size_t vga_line_range_min;
static size_t vga_line_range_max;

static uint8_t vga_color;
static bool disable_scroll = false;
static bool state_lock = false;

static void (*vga_scroll_cb)();

static void vga_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
  const size_t index = y * VGA_WIDTH + x;
  vga_buffer[index] = vga_entry(c, color);
}

void vga_putentry(uint16_t entry, size_t row, size_t col) {
  const size_t index = row * VGA_WIDTH + col;
  vga_buffer[index] = entry;
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
  int8_t scroll = (vga_cursor.row < vga_line_range_min) ? -1 : 1;
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

// Ref: http://www.osdever.net/FreeVGA/vga/crtcreg.htm#0E
static void vga_update_cursor() {
  uint16_t pos = vga_cursor.row * VGA_WIDTH + vga_cursor.col;
  outb(0x3D4, 0x0F);
  outb(0x3D5, (pos & 0xFF));
  outb(0x3D4, 0x0E);
  outb(0x3D5, ((pos >> 8) & 0xFF));
}
void vga_set_cursor_pos(size_t row, size_t col) {
  vga_cursor.row = row, vga_cursor.col = col;
  vga_update_cursor();
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
  vga_max_lines = VGA_HEIGHT - 1;
  vga_line_range_min = 0;
  vga_line_range_max = vga_max_lines;
  vga_set_cursor_block();
}

bool vga_must_scroll() {
  return (vga_cursor.row < vga_line_range_min ||
          vga_cursor.row > vga_line_range_max);
}
void vga_putchar(char c) {
  unsigned char uc = c;

  if (uc == '\r') {
    vga_cursor.col = 0;
  } else if (uc == '\n') {
    vga_cursor.col = 0;
    vga_cursor.row++;
    vga_total_lines++;
  } else {
    vga_putentryat(uc, vga_color, vga_cursor.col, vga_cursor.row);
    vga_cursor.col++;
  }
  if (vga_cursor.col == VGA_WIDTH) {
    vga_cursor.col = 0;
    vga_cursor.row++;
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

void vga_set_scroll_cb(void (*cb)()) {
  vga_scroll_cb = cb;
}

void vga_mem_cpy(size_t row, size_t col, uint16_t* src, size_t size) {
  memcpy(VGA_MEMORY + (row * VGA_WIDTH + col), src, size * 2);
}
