#include <stdint.h>
#include <string.h>

#include <kernel/idt.h>

struct idt_entry idt[256];
struct idt_ptr idtp;

void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
  idt[num].offset_1 = base >> 16;              // higher nibbled
  idt[num].offset_2 = base & ((2 << 16) - 1);  // lower nibbles
  idt[num].selector = sel;
  idt[num].zero = 0;
  idt[num].type_attributes = flags;
}

void idt_install() {
  idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
  idtp.base = (uint32_t)&idt;

  // Init to 0s
  memset(&idt, 0, sizeof(struct idt_entry) * 256);

  // idt_set_gate used here for new ISRs

  idt_load();
}
