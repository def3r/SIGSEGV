#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <stdint.h>

struct regs {
  uint32_t gs, fs, es, ds;
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uint32_t int_no, err_code;
  uint32_t eip, cs, eflags, userresp, ss;
} __attribute__((packed));

static inline void outb(uint16_t port, uint8_t val) {
  __asm__ __volatile__("outb %b0, %w1" ::"a"(val), "Nd"(port) : "memory");
}

static inline uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm__ __volatile__("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
  return ret;
}

void timer_install();

#endif  // !SYSTEM_H_
