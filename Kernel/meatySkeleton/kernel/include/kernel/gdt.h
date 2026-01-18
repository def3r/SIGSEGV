#ifndef _GDT_H
#define _GDT_H

#include <stdint.h>

extern void setGdt(uint16_t limit, uint32_t base);

struct GDT {
  uint32_t base;
  uint32_t limit;
  uint8_t access_byte;
  uint8_t flags;
} __attribute__((packed));

void gdt_init(void);

#endif  // !_GDT_H
