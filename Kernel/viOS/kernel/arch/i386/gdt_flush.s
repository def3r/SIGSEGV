# We use .global to make the function visible to the C code
.global setGdt

.section .text
setGdt:
    # Get the limit (first argument) from the stack
    # 4(%esp) is the address of the first argument
    mov 4(%esp), %ax
    mov %ax, (gdtr)

    # Get the base address (second argument)
    mov 8(%esp), %eax
    mov %eax, (gdtr + 2)

    # Load the GDT
    lgdt (gdtr)

    # Reload Data Segments
    # In GAS, we use $ for immediate values
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    # The Far Jump to reload Code Segment (CS)
    # 0x08 is the segment, and the .flush label is the offset
    ljmp $0x08, $.flush

.flush:
    ret

.section .data
# Aligning on 4 bytes to prevent issues
.align 4
gdtr:
    .word 0   # 16-bit limit
    .long 0   # 32-bit base address
