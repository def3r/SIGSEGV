#include <stdint.h>
#include <stdio.h>

// Replace string 111 by 101

enum State { Q0, Q1, Q2, Q3, Q4, Q5 };
enum Dir { LEFT = -1, RIGHT = 1, NOMOVE = 0 };

typedef struct {
  char *tape;
  enum State state;
  enum Dir dir;
  char *head;
} turing_machine;

const enum State final_state = Q5;

void transition_func(turing_machine *tm) {
  switch (tm->state) {
  case Q0:
    tm->dir = RIGHT;
    if (*tm->head == '1')
      tm->state = Q1;
    else if (*tm->head == '\0')
      tm->state = Q5, tm->dir = NOMOVE;
    break;

  case Q1:
    tm->dir = RIGHT;
    if (*tm->head == '1')
      tm->state = Q2;
    else if (*tm->head == '0')
      tm->state = Q0;
    else if (*tm->head == '\0')
      tm->state = Q5, tm->dir = NOMOVE;
    break;

  case Q2:
    tm->dir = RIGHT;
    if (*tm->head == '1')
      tm->state = Q3, tm->dir = LEFT;
    else if (*tm->head == '0')
      tm->state = Q0;
    else if (*tm->head == '\0')
      tm->state = Q5, tm->dir = NOMOVE;
    break;

  case Q3:
    *tm->head = '0';
    tm->dir = RIGHT;
    tm->state = Q4;
    break;

  case Q4:
    tm->state = Q0;
    break;

  case Q5:
    return;
  }

  tm->head += tm->dir;
}

void exec(turing_machine *tm) {
  while (tm->state != final_state) {
    transition_func(tm);
  }
}

int main() {
  char buff[BUFSIZ] = "111001011111";
  turing_machine tm = {.tape = buff, .state = Q0, .dir = RIGHT};
  tm.head = tm.tape;
  printf("Input Tape: %s\n", tm.tape);
  exec(&tm);
  printf("Output: %s\n", tm.tape);
}
