#!/bin/sh
set -e
. ./iso.sh

# -display curses # to display in terminal ui
qemu-system-$(./target-triplet-to-arch.sh $HOST) -display sdl -cdrom vios.iso
# -monitor stdio # then i/p: info registers
