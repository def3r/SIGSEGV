#include <stdio.h>
#include <string.h>

// DFA for a string ending w/ ab
// sigma = {a, b}
//
//    +-b-v       +-a-v
// -> [ q0 ] -a-> [ q1 ] -b-> [[ q2 ]]
//       ^           ^----a-----+  +
//       +------------b------------+

char string[BUFSIZ];
size_t string_len = 0;
#define STRING(str)                                                            \
  string_len = sizeof(str);                                                    \
  memcpy(string, str, string_len)

enum State { Q0, Q1, Q2 };
const enum State finalState = Q2;
const enum State initState = Q0;

enum State trans_fn(enum State state, char symbol) {
  switch (state) {
  case Q0:
    return (symbol == 'a') ? Q1 : Q0;
  case Q1:
    return (symbol == 'b') ? Q2 : Q1;
  case Q2:
    return (symbol == 'a') ? Q1 : Q0;
  }
  return 0;
}

int main() {
  STRING("abaababab");
  printf("STRING: %s\n", string);

  enum State state = initState;
  for (int idx = 0; idx < string_len - 1; idx++) {
    state = trans_fn(state, string[idx]);
  }

  printf("Output: %sAccepted\n", (state == finalState) ? "" : "Not ");
}
