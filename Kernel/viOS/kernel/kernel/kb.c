#include <kb.h>
#include <stdint.h>
#include <stdio.h>
#include <system.h>

#include <arch/idt.h>
#include <kernel/terminal.h>

// BITS:
// ALT|CTRL|SHIFT|-|-|CAPSLOCK|NUMLOCK|SCROLLLOCK
//  7   6     5          2         1      0
uint8_t key_status = 0;

// Src: http://www.osdever.net/bkerndev/Docs/keyboard.htm
//
// clang-format off
/* KBDUS means US Keyboard Layout. This is a scancode table
*  used to layout a standard US keyboard. I have left some
*  comments in to give you an idea of what key is what, even
*  though I set it's array index to 0. You can change that to
*  whatever you want using a macro, if you wish! */
unsigned char kbdus[2][128] =
{
  {
      0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
    '9', '0', '-', '=', '\b',	/* Backspace */
    '\t',			/* Tab */
    'q', 'w', 'e', 'r',	/* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13,	/* Enter key */
      0,			/* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
   '\'', '`',   0,		/* Left shift */
   '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
    'm', ',', '.', '/',   0,				/* Right shift */
    '*',
      0,	/* Alt */
    ' ',	/* Space bar */
      0,	/* Caps lock */
      0,	/* 59 - F1 key ... > */
      0,   0,   0,   0,   0,   0,   0,   0,
      0,	/* < ... F10 */
      0,	/* 69 - Num lock*/
      0,	/* Scroll Lock */
      0,	/* Home key */
      0,	/* Up Arrow */
      0,	/* Page Up */
    '-',
      0,	/* Left Arrow */
      0,
      0,	/* Right Arrow */
    '+',
      0,	/* 79 - End key*/
      0,	/* Down Arrow */
      0,	/* Page Down */
      0,	/* Insert Key */
      0,	/* Delete Key */
      0,   0,   0,
      0,	/* F11 Key */
      0,	/* F12 Key */
      0,	/* All other keys are undefined */
  },

  // with shift pressed
  {
      0,  27, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
    '(', ')', '_', '+', '\b',	/* Backspace */
    '\t',			/* Tab */
    'Q', 'W', 'E', 'R',	/* 19 */
    'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 13,	/* Enter key */
      0,			/* 29   - Control */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
   '\"', '~',   0,		/* Left shift */
    '|', 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
    'M', '<', '>', '?',   0,				/* Right shift */
    '*',
      0,	/* Alt */
    ' ',	/* Space bar */
      0,	/* Caps lock */
      0,	/* 59 - F1 key ... > */
      0,   0,   0,   0,   0,   0,   0,   0,
      0,	/* < ... F10 */
      0,	/* 69 - Num lock*/
      0,	/* Scroll Lock */
      0,	/* Home key */
      0,	/* Up Arrow */
      0,	/* Page Up */
    '-',
      0,	/* Left Arrow */
      0,
      0,	/* Right Arrow */
    '+',
      0,	/* 79 - End key*/
      0,	/* Down Arrow */
      0,	/* Page Down */
      0,	/* Insert Key */
      0,	/* Delete Key */
      0,   0,   0,
      0,	/* F11 Key */
      0,	/* F12 Key */
      0,	/* All other keys are undefined */
  }
};
// clang-format on

#define LSHIFT 0x2A
#define LSHIFT_BIT (1 << 5)
#define ISLSHIFT(x) (x & LSHIFT_BIT) == LSHIFT_BIT

#define KEY_RELEASED(x) x & 0x80
#define RELEASED(x) x | 0x80

void keyboard_handler(struct regs* r) {  // NOLINT
  uint8_t scancode;
  scancode = inb(0x60);

  // Key released, 7th bit set
  if (KEY_RELEASED(scancode)) {
    switch (scancode) {
      case RELEASED(LSHIFT):
        key_status &= ~LSHIFT_BIT;
        break;
    }
    return;
  }

  switch (scancode) {
    case LSHIFT:
      key_status |= LSHIFT_BIT;
      break;
    default: {
      char c = kbdus[ISLSHIFT(key_status)][scancode];
      // printf("%d", scancode);
      handle_input(c);
    }
  }
}

void kb_install() {
  irq_install_handler(1, keyboard_handler);
}
