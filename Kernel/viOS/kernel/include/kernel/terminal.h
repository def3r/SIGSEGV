#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <stdint.h>

void handle_input(uint8_t in);
void term_init();
void term_update();

#endif  // !TERMINAL_H_
