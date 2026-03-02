#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <stdint.h>

struct regs {
  uint32_t gs, fs, es, ds;
  uint32_t dummy, edi, esi, ebp, esp, ebx, edx, ecx,
      eax;  // what is dummy here?
            // without it int_no and err_code are shifter 4 bytes off their
            // struct pos!
  uint32_t int_no, err_code;
  uint32_t eip, cs, eflags, userresp, ss;
} __attribute__((packed));

#endif  // !SYSTEM_H_
