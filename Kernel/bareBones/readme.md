`https://wiki.osdev.org/Bare_Bones#Booting_the_Operating_System`

gcc cross compiler @ `~/opt/cross/` / i686-elf @ `~/opt/cross/i686-elf/bin/`

Assemble boot.s
```console
$ i686-elf-as boot.s -o boot.o
```

Compile the kernel
```console
$ i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
```

Linking the kernel
```console
$ i686-elf-gcc -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc
```

To check whether grub could find the multiboot header in first 8KiB
```console
$ grub-file --is-x86-multiboot myos.bin  # exits silently if passed, else $? set
```

To make cdrom image, first create dirs: isodir/boot/grub/
with kernel in boot/ and grub.cfg in boot/grub
```console
$ grub-mkrescue -o myos.iso isodir
```

Boot the iso
```console
$ qemu-system-i386 -cdrom myos.iso
```

Boot the kernel
```console
$ qemu-system-i386 -kernel myos.bin
```

## Keyboard Support
-> Need Interrupts -> Need Interrupt Descriptor Table -> Need Global Descriptor Table

### Loading the GDT:
`https://wiki.osdev.org/GDT_Tutorial`

1. Paging is currently disabled
2. Assumed im non-flat (base != 0)
3. Hardcoded the gdt entries
4. Switched from AT&T to NASM
