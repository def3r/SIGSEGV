#ifndef KERNEL_TTY_H_
#define KERNEL_TTY_H_

#include <stddef.h>
#include <stdint.h>

void vga_init(void);
void vga_putchar(char c);
void vga_write(const char* data, size_t size);
void vga_writestring(const char* data);

void vga_cursor_left(size_t n);
void vga_cursor_right(size_t n);
void vga_cursor_up(size_t n);
void vga_cursor_down(size_t n);

void vga_set_cursor_block();
void vga_set_cursor_underline();

#endif  // !KERNEL_TTY_H_
