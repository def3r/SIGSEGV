#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <kernel/gdt.h>

void encodeGdtEntry(uint8_t *target, struct GDT source) {
  // Check the limit to make sure that it can be encoded
  if (source.limit > 0xFFFFF) {
    printf("GDT cannot encode limits larger than 0xFFFFF");
    abort();
  }

  // Encode the limit
  target[0] = source.limit & 0xFF;
  target[1] = (source.limit >> 8) & 0xFF;
  target[6] = (source.limit >> 16) & 0x0F;

  // Encode the base
  target[2] = source.base & 0xFF;
  target[3] = (source.base >> 8) & 0xFF;
  target[4] = (source.base >> 16) & 0xFF;
  target[7] = (source.base >> 24) & 0xFF;

  // Encode the access byte
  target[5] = source.access_byte;

  // Encode the flags
  target[6] |= (source.flags << 4);
}

__attribute__((aligned(8))) uint8_t gdt_entries[24];

void gdt_init(void) {
  // 1. Encode Null Segment (all zeros)
  encodeGdtEntry(
      &gdt_entries[0],
      (struct GDT){.base = 0, .limit = 0, .access_byte = 0, .flags = 0});

  // 2. Encode Kernel Code Segment (Offset 0x08)
  encodeGdtEntry(&gdt_entries[8], (struct GDT){.base = 0,
                                               .limit = 0xFFFFF,
                                               .access_byte = 0x9A,
                                               .flags = 0x0C});

  // 3. Encode Kernel Data Segment (Offset 0x10)
  encodeGdtEntry(&gdt_entries[16], (struct GDT){.base = 0,
                                                .limit = 0xFFFFF,
                                                .access_byte = 0x92,
                                                .flags = 0x0C});

  // Limit is (Total Bytes - 1)
  setGdt(23, (uint32_t)&gdt_entries);
}
