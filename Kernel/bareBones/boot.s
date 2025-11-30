# Constants
.set ALIGN,    1<<0
.set MEMINFO,  1<<1
.set FLAGS,    ALIGN | MEMINFO
.set MAGIC,    0x1BADB002
.set CHECKSUM, -(MAGIC + FLAGS)

# Multiboot header, marks prog as kernel
# magic nums
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Create stack
# Multiboot does not provide a stack pointer by itself
# Kernel's duty to create esp
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# _start is the entry point of the kernel
# since bootloader is gone, no reason to return from this func
.section .text
.global _start
.type   _start, @function
_start:
    # WE ARE IN! Bootloader loaded us in 32-bit protected mode on x86
    # Interrupts are disabled
    # Paging is disabled
    # Proc state defined in multiboot std
    # Kernel has full control of CPU!
    # Kernel can onle make use of hardware features
    # No printf, until kernel provides a stdio.h
    # No security restrictions
    # No debugging mechanism

    # set the esp register
    mov $stack_top, %esp    # $-> Immediate val, %-> register

    # Processeor State Initialization here
    # Processor not fully initialized yet
    # Global Descriptor Table (GDT) must be loaded here
    # Paging enabled here

    # high level kernel
    call kernel_main

    # if system has nothing to do, we put an infinite loop
    # 1. Disable interrupts (clear interrupt enable in eflags)
    cli

    # 2. Wait for next interrupt to arrive
1:  hlt

    # 3. If it ever wakes up (due to non-maskable interrupts), jump to halt
    jmp 1b  // 1b -> label 1, backwards, (1f exists for forward)

.size _start, . - _start
