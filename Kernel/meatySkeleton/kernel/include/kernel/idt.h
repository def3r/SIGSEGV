#ifndef IDT_H_
#define IDT_H_

#include <stdint.h>

// clang-format off
struct idt_entry {
  uint16_t  offset_1;        // offset bits 0..15
  uint16_t  selector;        // a code segment selector in GDT or LDT
  uint8_t   zero;            // unused, set to 0
  uint8_t   type_attributes; // gate type, dpl, and p fields
  uint16_t  offset_2;        // offset bits 16..31
} __attribute__((packed));
// clang-format on

struct idt_ptr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

extern void idt_load();
void idt_install();

#endif  // !IDT_H+
