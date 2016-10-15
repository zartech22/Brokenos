SRC= $(wildcard *.c)
OBJ= $(SRC:.c=.o)

#CC=g++
#CFLAGS=-m32 -std=c++11 -c -fno-exceptions -nostdlib -ffreestanding -fno-rtti
CC=gcc
CFLAGS=-Wall -fno-builtin -fno-stack-protector -nostdlib -ffreestanding -m32

LD=ld
LDFLAGS=-m elf_i386 -Ttext=40000000 --entry=main

OBJECTS= $(wildcard build/*.o)

all: $(basename $(OBJ))

$(basename $(OBJ)): $(OBJ)
	$(LD) $(LDFLAGS) -o build/$(basename $@) build/$@.o
	echo $*

%.o: %.c
	$(CC) $(CFLAGS) -o build/$@ -c $<
