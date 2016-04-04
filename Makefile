CC=g++
CFLAGS=-m32 -std=c++11 -fno-stack-protector -c -fno-exceptions -ffreestanding
ASM=nasm
AFLAGS=-f elf -o
LDFLAGS=-m elf_i386 -Ttext 100000 --entry=_start
EXEC=link
SRC= $(wildcard *.c)
OBJ= $(SRC:.c=.o)

all: $(EXEC)

run: clean all
	qemu-system-i386 -fda $(EXEC)

vmware: all
	sudo cp kernel /media/kevin/3842f66d-4707-4554-a3e4-e6fd7c78dcbd/boot/
	sync
	@sudo -i vmplayer /home/kevin/vmware/Kernel/Kernel.vmx &> /dev/null &

floppyA: link
	cat bootsect kernel /dev/zero | dd of=$@ bs=512 count=2880

link: $(OBJ) ide.o int.o do_switch.o boot.o icxxabi.o Ext2FS.o
	ld $(LDFLAGS) build/boot.o build/icxxabi.o build/kernel.o build/disk.o build/pci.o build/Ext2FS.o build/ide.o build/kmalloc.o build/gdt.o build/idt.o build/int.o build/interrupt.o build/lib.o build/mm.o build/pic.o build/strLib.o build/do_switch.o build/scheduler.o build/process.o  -o kernel

%.o: %.asm
	$(ASM) $(AFLAGS) build/$@ $<

%.o: %.c
	$(CC) $(CFLAGS) -o build/$@ $^
%.o: %.cpp
	$(CC) $(CFLAGS) -o build/$@ $^

clean:
	@rm -f build/*.o