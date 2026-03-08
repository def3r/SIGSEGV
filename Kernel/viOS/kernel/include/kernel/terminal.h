#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <stddef.h>
#include <stdint.h>

void term_write(const char* data, size_t size);

void handle_input(uint8_t in);
void term_init();
void term_update();

#endif  // !TERMINAL_H_
