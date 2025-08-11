#include <curses.h>

// Source
// https://tldp.org/HOWTO/Text-Terminal-HOWTO-16.html
// https://man7.org/linux/man-pages/man5/terminfo.5.html
// https://unix.stackexchange.com/questions/573410/how-to-interact-with-a-terminfo-database-in-c-without-ncurses

// WORDS OF WISDOM:
//  "
//    The idea that terminfo/termcap is more portable is really stretched to the
//    breaking point and beyond in the second decade of the 21st century. You
//    almost certainly won't ever encounter a real video terminal that isn't
//    ECMA-48:1976 conformant, let alone a paper terminal. And the terminfo
//    abstractions, which often do not actually match what real video terminals
//    do, are in some ways an impediment to portability as they force a somewhat
//    contorted way of working.
//  "

// Funfact: nvim uses https://github.com/mauke/unibilium to parse terminfo

// Terminfo
// * The current terminal's capabilities database can be accesed by the command
//   $ infocmp
// * `$ tic` is the terminal info compiler, which translates terminfo & termcap
//    formats. See $ man tic
//
// Terminfo (formerly Termcap) is a database of terminal capabilities and more.
// For every (well almost) model of terminal it tells application programs what
// the terminal is capable of doing
// t tells what escape sequences (or control characters) to send to the terminal
// in order to do things such as move the cursor to a new location, erase part
// of the screen, scroll the screen, change modes, change appearance
//
// The terminfo database is compiled and thus has a source part and a compiled
// part.

int main() {}
