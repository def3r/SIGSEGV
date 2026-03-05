#ifndef KERNEL_TTY_H_
#define KERNEL_TTY_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
  size_t x, y;
} vga_pos;

void vga_init(void);
void vga_putchar(char c);
void vga_write(const char* data, size_t size);
void vga_writestring(const char* data);

void vga_cursor_left(size_t n);
void vga_cursor_right(size_t n);
void vga_cursor_up(size_t n);
void vga_cursor_down(size_t n);

vga_pos vga_get_cursor_pos();
void vga_set_cursor_pos(size_t x, size_t y, bool scroll2view);
void vga_set_cursor_block();
void vga_set_cursor_underline();

void vga_set_max_lines(size_t n);
uint8_t vga_screen_height();
uint8_t vga_screen_width();
void vga_disable_scroll();
void vga_enable_scroll();

void vga_set_color_from(enum vga_color fg, enum vga_color bg);
void vga_set_color(uint8_t color);
uint8_t vga_get_color();

void vga_get_cur_memline_range(size_t res[2]);
void vga_memline_fill_with(size_t x, size_t y, char c);
void vga_cur_memline_fill_with(char c);
void vga_swap_memline(size_t row1, size_t row2);
size_t vga_get_total_lines();

void vga_set_scroll_cb(void (*cb)());

#endif  // !KERNEL_TTY_H_
