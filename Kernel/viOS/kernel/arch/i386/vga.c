#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/vga.h>

#include "system.h"
#include "vga.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint32_t const VGA_MEMORYLOC = 0xB8000;
static uint16_t* const VGA_MEMORY = (uint16_t*)VGA_MEMORYLOC;

#define DEC(x, n) (x <= n) ? 0 : x - n

typedef enum vga_cursor_types { UNDERLINE, BLOCK } CursorTypes;

typedef struct vga_pos {
  size_t x, y;
} vga_pos;

static vga_pos vga_cursor;
static size_t vga_offset;
static uint8_t vga_color;
static uint16_t* vga_buffer;
static size_t vga_max_lines;

static void vga_setcolor(uint8_t color) {
  vga_color = color;
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
  vga_offset++;
  outb(0x3D4, 0x0D);
  outb(0x3D5, (vga_offset * VGA_WIDTH & 0xFF));
  outb(0x3D4, 0x0C);
  outb(0x3D5, ((vga_offset * VGA_WIDTH >> 8) & 0xFF));
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
  vga_set_cursor_block();
}

void vga_putchar(char c) {
  unsigned char uc = c;

  if (uc == '\r') {
    vga_cursor.y = 0;
  } else if (uc == '\n') {
    vga_cursor.y = 0;
    vga_cursor.x++;
  } else {
    vga_putentryat(uc, vga_color, vga_cursor.y, vga_cursor.x);
    vga_cursor.y++;
  }
  if (vga_cursor.y == VGA_WIDTH) {
    vga_cursor.y = 0;
    vga_cursor.x++;
  }

  // Actually scroll through the memory
  if (vga_cursor.x > VGA_HEIGHT + vga_offset - 1) {
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
  size_t offset = DEC(vga_cursor.x, VGA_HEIGHT);
  if (offset != vga_offset &&
      (offset / VGA_HEIGHT) != (vga_offset / VGA_HEIGHT)) {
    vga_offset = offset;
    vga_scroll2();
  }
  vga_update_cursor();
}

void vga_cursor_down(size_t n) {
  vga_cursor.x = MIN(vga_cursor.x + n, VGA_HEIGHT + vga_offset - 1);
  size_t offset = DEC(vga_cursor.x, VGA_HEIGHT - 1);
  if (offset != vga_offset &&
      (offset / VGA_HEIGHT) != (vga_offset / VGA_HEIGHT)) {
    vga_offset = offset;
    vga_scroll2();
  }
  vga_update_cursor();
}
