#ifndef KERNEL_TTY_H_
#define KERNEL_TTY_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
static uint32_t const VGA_MEMORYLOC = 0xB8000;
static uint16_t* const VGA_MEMORY = (uint16_t*)VGA_MEMORYLOC;

enum vga_color {
  VGA_COLOR_BLACK = 0,
  VGA_COLOR_BLUE = 1,
  VGA_COLOR_GREEN = 2,
  VGA_COLOR_CYAN = 3,
  VGA_COLOR_RED = 4,
  VGA_COLOR_MAGENTA = 5,
  VGA_COLOR_BROWN = 6,
  VGA_COLOR_LIGHT_GREY = 7,
  VGA_COLOR_DARK_GREY = 8,
  VGA_COLOR_LIGHT_BLUE = 9,
  VGA_COLOR_LIGHT_GREEN = 10,
  VGA_COLOR_LIGHT_CYAN = 11,
  VGA_COLOR_LIGHT_RED = 12,
  VGA_COLOR_LIGHT_MAGENTA = 13,
  VGA_COLOR_LIGHT_BROWN = 14,
  VGA_COLOR_WHITE = 15,
};

typedef struct vga_pos {
  size_t row, col;
} vga_pos;

inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
  return (uint16_t)uc | (uint16_t)color << 8;
}
void vga_putentry(uint16_t entry, size_t row, size_t col);
void vga_mem_cpy(size_t row, size_t col, uint16_t* src, size_t size);
inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
  return fg | bg << 4;
}

void vga_init(void);
void vga_putchar(char c);
void vga_write(const char* data, size_t size);
void vga_writestring(const char* data);

void vga_set_cursor_pos(size_t row, size_t col);
void vga_set_cursor_block();
void vga_set_cursor_underline();

void vga_set_scroll_cb(void (*cb)());

#endif  // !KERNEL_TTY_H_
