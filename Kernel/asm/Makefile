ASM=nasm
FLAGS=-g -f elf -o

SRC= $(wildcard *.asm)
OBJ= $(SRC:.asm=.o)

all: $(OBJ)

%.o : %.asm
	$(ASM) $(FLAGS) ../build/$@ $<
