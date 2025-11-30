/* Ned a customized "linker script" */

/* entry point to tell bootloader */
ENTRY(_start)

/* To tell where different sections of kernel will be put in the final */
/* kernel image! */
SECTIONS {
   /* Start offset */
  . = 2M;

   /* First thing in our image is the multiboot header */
   /* For grub to load it, header must be in the first 8KiB of kernel, @4B alignment*/
  .text BLOCK(4K) : ALIGN(4K) {
    *(.multiboot)
    *(.text)
  }

   /* Read only data */
  .rodata BLOCK(4K) : ALIGN(4K) {
    *(.rodata)
  }

   /* Read-Write data (initialized) */
  .data BLOCK(4K) : ALIGN(4K) {
    *(.data)
  }

  .bss BLOCK(4K) : ALIGN(4K) {
    *(COMMON)
    *(.bss)
  }

   /* Compilers may produce other sections */
   /* by default, put them in a segment with the same name */
}
