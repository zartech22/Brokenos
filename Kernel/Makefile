CC=g++
CFLAGS=-m32 -std=c++11 -g -c -o0 -fno-exceptions -nostdlib -ffreestanding -fno-rtti -I..

LD=ld
LDFLAGS=-m elf_i386 -T build/linker.ld

FIND_COMMAND=find build/ -type f -name "*.o"

OBJECTS= $(shell $(FIND_COMMAND))

export CC
export CFLAGS

all: link

link: compile
	$(LD) $(LDFLAGS) $(OBJECTS)

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
	$(FIND_COMMAND) -delete
