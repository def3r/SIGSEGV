#include <stdio.h>
#include <termios.h>
#include <unistd.h>

// termios.h
//
// Terminal inputs can be handled using this library.
// There is so much to just how shells manage terminals.
//
// One can enable RAW Input from the user by disabling
// what is called Cannonical mode, which stores the input
// in the terminal buffer and releases it only when enter
// is pressed. But by disabling this, one gets raw input
// from the user as soon as the key is pressed!
//
// FYI: tty (teletypewriter) in modern terms is a terminal.

int main() {
  struct termios new_t, old_t;

  tcgetattr(STDIN_FILENO, &old_t);
  new_t = old_t;

  // Canonical Mode: ICANON : Refers to line buffering in tty
  // Echo Mode: ECHO : Refers to whether the characters input
  //                   are visible on terminal
  new_t.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_t);

  char c;
  printf("Press 'q' to quit!\n");
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {
    printf("pressed: %c\n", c);
  }

  // The issue with restoring the original terminal attributes
  // at the end of the program is the wishful thinking that the
  // program will always execute completely.
  //
  // In case of any signal like SIGINT (Ctrl + C) etc, this would
  // not be reset and cause inconvenience to the user.
  //
  // thus we handle those signals exclusively or using: atexit()
  tcsetattr(STDIN_FILENO, TCSANOW, &old_t);
  return 0;
}
