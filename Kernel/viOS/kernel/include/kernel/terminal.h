#ifndef TERMINAL_H_
#define TERMINAL_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum TermMode { NORMAL = 0, INSERT = 1 } TermMode;
typedef enum Direction { FWD = 1, BWD = -1 } Direction;

void term_write(const char* data, size_t size);

void term_set_cursor_block();
void term_set_cursor_underline();
void term_inschar(char in);

void term_skip_word(Direction dir, bool (*cmp)(const char));
void term_skip_short_word(Direction dir);
void term_skip_long_word(Direction dir);
void term_skip_to_last_char();
void term_cursor_left(size_t n);
void term_cursor_right(size_t n);
void term_cursor_up(size_t n);
void term_cursor_down(size_t n);
void term_skip_to_first_char();
void term_del_char(Direction dir);

void term_init();
void term_update();
TermMode term_get_mode();
void term_set_mode(TermMode m);

#endif  // !TERMINAL_H_
