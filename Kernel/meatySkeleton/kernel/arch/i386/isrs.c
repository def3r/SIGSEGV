#include <kernel/idt.h>
#include <stdint.h>
#include <stdio.h>
#include <system.h>

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

void example0() {
  printf("This");
  for (;;)
    ;
}

void isrs_install() {
  // Access flags = 0x8E -> entry is present and in ring 0 (kernel mode) and
  // lower 5 bits set to required 14 (hex E)
  idt_set_gate(0, (unsigned)isr0, 0x08, 0x8E);
  // idt_set_gate(1, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(2, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(3, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(4, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(5, (unsigned)example0, 0x08, 0x8E);
  idt_set_gate(6, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(7, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(8, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(9, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(10, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(11, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(12, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(13, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(14, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(15, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(16, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(17, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(18, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(19, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(20, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(21, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(22, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(23, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(24, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(25, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(26, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(27, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(28, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(29, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(30, (unsigned)example0, 0x08, 0x8E);
  // idt_set_gate(31, (unsigned)example0, 0x08, 0x8E);
}

const unsigned char* exception_msg[] = {
    (unsigned char*)"Division By Zero Exception",
    (unsigned char*)"Debug Exception",
    (unsigned char*)"Non Maskable Interrupt Exception",
    (unsigned char*)"Breakpoint Exception",
    (unsigned char*)"Into Detected Overflow Exception",
    (unsigned char*)"Out of Bounds Exception",
    (unsigned char*)"Invalid Opcode Exception",
    (unsigned char*)"No Coprocessor Exception",
    (unsigned char*)"Double Fault Exception",
    (unsigned char*)"Coprocessor Segment Overrun Exception",
    (unsigned char*)"Bad TSS Exception",
    (unsigned char*)"Segment Not Present Exception",
    (unsigned char*)"Stack Fault Exception",
    (unsigned char*)"General Protection Fault Exception",
    (unsigned char*)"Page Fault Exception",
    (unsigned char*)"Unknown Interrupt Exception",
    (unsigned char*)"Coprocessor Fault Exception",
    (unsigned char*)"Alignment Check Exception (486+)",
    (unsigned char*)"Machine Check Exception (Pentium/586+)",
    (unsigned char*)"Reserved Exception",
    (unsigned char*)"Reserved Exception",
    (unsigned char*)"Reserved Exception",
    (unsigned char*)"Reserved Exception",
    (unsigned char*)"Reserved Exception",
    (unsigned char*)"Reserved Exception",
    (unsigned char*)"Reserved Exception",
    (unsigned char*)"Reserved Exception",
    (unsigned char*)"Reserved Exception",
    (unsigned char*)"Reserved Exception",
    (unsigned char*)"Reserved Exception",
    (unsigned char*)"Reserved Exception",
    (unsigned char*)"Reserved Exception",
};

void fault_handler(struct regs* r) {
  printf("fault number: %d: ", r->int_no);
  printf("fault return code %d: ", r->err_code);
  // printf("%d %d %d %d %d",
  //        r->gs, r->fs, r->es, r->ds, r->dummy);
  // r->edi, r->esi, r->ebp, r->esp, r->ebx, r->edx, r->ecx, r->eax);
  // r->int_no, r->err_code);
  // r->eip, r->cs, r->eflags, r->userresp, r->ss);
  if (r->int_no < 32) {
    puts((char*)exception_msg[r->int_no]);
    printf("Exception occured! System Halt!");
  for (;;)
    ;
   }
  printf("fault handler");
}
