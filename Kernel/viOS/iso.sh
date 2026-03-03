#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/vios.kernel isodir/boot/vios.kernel
cat >isodir/boot/grub/grub.cfg <<EOF
menuentry "vi OS" {
	multiboot /boot/vios.kernel
}
EOF
grub-mkrescue -o vios.iso isodir
