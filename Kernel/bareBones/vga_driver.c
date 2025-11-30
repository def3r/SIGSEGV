// This driver does not run in a hosted environment
// Rather it runs in a freestanding environment
//  => No C Std lib 4u

// All non std libs btw
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// #if defined(__linux__)
// #error "Need to use a cross compiler \
// https://wiki.osdev.org/GCC_Cross-Compiler" #endif
//
// #if !defined(__i386__)
// #error "Need to compile with ix86-elf compiler"
// #endif

// clang-format off
/* Hardware text mode color constants. */
enum vga_color {
  VGA_COLOR_BLACK         = 0,
  VGA_COLOR_BLUE          = 1,
  VGA_COLOR_GREEN         = 2,
  VGA_COLOR_CYAN          = 3,
  VGA_COLOR_RED           = 4,
  VGA_COLOR_MAGENTA       = 5,
  VGA_COLOR_BROWN         = 6,
  VGA_COLOR_LIGHT_GREY    = 7,
  VGA_COLOR_DARK_GREY     = 8,
  VGA_COLOR_LIGHT_BLUE    = 9,
  VGA_COLOR_LIGHT_GREEN   = 10,
  VGA_COLOR_LIGHT_CYAN    = 11,
  VGA_COLOR_LIGHT_RED     = 12,
  VGA_COLOR_LIGHT_MAGENTA = 13,
  VGA_COLOR_LIGHT_BROWN   = 14,
  VGA_COLOR_WHITE         = 15,
};
// clang-format on

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
  return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
  return (uint16_t)uc | (uint16_t)color << 8;
}

size_t strlen(const char *str) {
  size_t len = 0;
  while (str[len++])
    ;
  return --len;
}

// clang-format off
#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  0xB8000 // VGA text mode buffer located at this loc
// clang-format on

size_t terminal_row;
size_t terminal_col;
uint8_t terminal_color;
uint16_t *terminal_buf = (uint16_t *)VGA_MEMORY;

void terminal_initialize(void) {
  terminal_row = 0;
  terminal_col = 0;
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = y * VGA_WIDTH + x;
      terminal_buf[index] = vga_entry(' ', terminal_color);
    }
  }
}

void terminal_setcolor(uint8_t color) { terminal_color = color; }

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
  const size_t index = y * VGA_WIDTH + x;
  terminal_buf[index] = vga_entry(c, color);
}

void terminal_putchar(char c) {
  if (c == '\n') {
    terminal_col = VGA_WIDTH - 1;
  } else {
    terminal_putentryat(c, terminal_color, terminal_col, terminal_row);
  }
  if (++terminal_col >= VGA_WIDTH) {
    terminal_col = 0;
    if (++terminal_row == VGA_HEIGHT) {
      terminal_row = 0;
    }
  }
}

void terminal_write(const char *data, size_t size) {
  for (size_t i = 0; i < size; i++)
    terminal_putchar(data[i]);
}

void terminal_writestring(const char *data) {
  terminal_write(data, strlen(data));
}

void terminal_clear(void) { terminal_initialize(); }

// NO WRAP

void terminal_putchar_nowrap(char c) {
  if (c == '\n') {
    terminal_col = VGA_WIDTH - 1;
  } else {
    terminal_putentryat(c, terminal_color, terminal_col, terminal_row);
  }
  if (++terminal_col >= VGA_WIDTH) {
    terminal_col = 0;
  }
}

void terminal_write_nowrap(const char *data, size_t size) {
  for (size_t i = 0; i < size; i++)
    terminal_putchar_nowrap(data[i]);
}

void terminal_writestring_nowrap(const char *data) {
  terminal_write_nowrap(data, strlen(data));
}

void terminal_yay_boo(void) {
  size_t col = 0;
  int max = 1e5;
  while (true) {
    for (float i = 0; i <= max; i += 0.1)
      ;
    terminal_clear();

    col++;
    col = (col % VGA_WIDTH);
    terminal_col = col;
    terminal_writestring_nowrap("YAY BOO! YAY BOO!");
  }
}

void kernel_main(void) {
  terminal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_CYAN);
  terminal_initialize();
  terminal_yay_boo();
}
