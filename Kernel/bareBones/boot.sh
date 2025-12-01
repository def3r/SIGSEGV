#!/bin/fish

# ~/opt/cross/$TARGET/bin/as boot.s -o boot.o
nasm -felf32 boot.nasm -o boot.o
$TARGET-gcc -c vga_driver.c -o vga_driver.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
$TARGET-gcc -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o vga_driver.o -lgcc
qemu-system-i386 -kernel myos.bin
# qemu-system-i386 -cdrom myos.iso
