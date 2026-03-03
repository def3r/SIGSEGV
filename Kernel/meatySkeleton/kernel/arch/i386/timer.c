#include <kernel/idt.h>
#include <stdint.h>
#include <stdio.h>
#include <system.h>

static uint32_t freq = 18;

void timer_phase(uint32_t hz) {  // NOLINT
  freq = hz;
  uint32_t divisor = 1193180 / hz;
  outb(0x43, 0x36);            // cmd byte = 36
  outb(0x40, divisor & 0xFF);  // lower bytes of divi
  outb(0x40, divisor >> 8);    // higher bytes of divi
}

uint32_t timer_ticks = 0;

void timer_handler(struct regs* r) {  // NOLINT
  // printf("aHacks: %d\n", timer_ticks);
  timer_ticks++;
  // Every 18 (by default) ticks ~1 second
  if (timer_ticks % freq == 0) {
    printf("%d seconds has passed!\n", (timer_ticks / freq));
  }
}

void timer_install() {
  irq_install_handler(0, timer_handler);
  // timer_phase(100);
}
