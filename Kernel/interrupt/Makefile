SRC= $(wildcard *.c)
OBJ= $(SRC:.c=.o)

all: $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -o ../build/$@ $^
