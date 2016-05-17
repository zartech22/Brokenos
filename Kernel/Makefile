CC=g++
CFLAGS=-m32 -std=c++11 -c -fno-exceptions -ffreestanding -fno-rtti -O2 -I..

LD=ld
LDFLAGS=-m elf_i386 -Ttext 100000 --entry=_start

export CC
export CFLAGS

all: link

link: compile
	$(LD) $(LDFLAGS) build/boot.o build/int32.o build/icxxabi.o build/kernel.o build/pci.o build/Ext2FS.o build/ide.o build/kmalloc.o build/gdt.o build/idt.o build/int.o build/interrupt.o build/lib.o build/mm.o build/pic.o build/Screen.o build/do_switch.o build/scheduler.o build/process.o -o kernel

compile:
	$(MAKE) -C asm
	$(MAKE) -C core
	$(MAKE) -C disk
	$(MAKE) -C interrupt
	$(MAKE) -C memory
	$(MAKE) -C pci
	$(MAKE) -C utils
	$(MAKE) -C video

clean:
	rm -rf build/*
