Following [Meaty Skeleton OSDev wiki](https://wiki.osdev.org/Meaty_Skeleton)

> We using GNU Assembly (GAS), it sucks a lil bit but will switch to NASM or
> something in the future.

#### Back to GDT ([Global Descriptor Table](https://wiki.osdev.org/GDT_Tutorial))
*resources*<br>
*http://www.osdever.net/bkerndev/Docs/gdt.htm*<br>
*https://youtu.be/5LbXClJhxcs?t=408*<br>
*https://stackoverflow.com/questions/23978486/far-jump-in-gdt-in-bootloader*<br>
*https://stackoverflow.com/questions/52490438/why-cant-mov-set-cs-the-code-segment-register-even-though-it-can-set-others*<br>

*Segmentation is a must*<br>
GDT handles segmentation, and this cannot be disabled, even if you want to use
paging.

*Segment Selectors= Segment Registers i.e. CS, DS, SS, ...*<br>
*Every segment selector is 16 bit sized.*
```
[   Index (13 bits)  ][T I][RPL]
15                   3  2  1   0

Request Privilege Level(0-1): 00 Highest, 11 Lowest
Table Indicator(2)          : 0 GDT, 1 LDT
Segment Index(3-15)
```

GDT must contain:
- Entry 0 in the DT, called the Null Descriptor, is never referenced by the
  processor and should always contain no data. (its 8 bytes wide)
- DPL 0 CS (Code Segment) Descriptor (for kernel)
- DS Descriptor
- Task State Segment descriptor
- Room for more segments, if needed.

- The CPU doesn't care where the GDT is (in memory)
- Can sit in the kernel's data section (global var in src code)
- To verify GDT loaded: use `-monitor stdio` flag w/ qemu and in terminal type:
  `info registers` and then check the DS/SS/CS and GDT registers

***The Story of GDT***<br>
At the time of booting, loader (`linker.ld`) will load the kernel-elf into the
memory as specified by `linker.ld`. CPU doesn't care where the GDT lives. It
wants a contiguous memory containing specific bits in order. So, we declare a
global var (this lives in the data section of kernel binary, loaded into RAM by
bootloader) array of gdt_entries, each of which will live as long as the kernel
is in memory. We encode the gdt entries at the memory location of array, and
now we need to tell the cpu that GDT is located at this address in memory.

Altho, `mov` can set the sections like DS, SS, ES etc. the CS is read only for mov.
Thus we have to `long jump` setting 0x08 to CS and Instruction Pointer to the
next label. This is important because the bootloader has its own GDT at the
time of booting, and if CS is not changed, we would still be using the
bootloader's GDT henceforth.

