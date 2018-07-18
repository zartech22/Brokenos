SRC= $(shell find ./ -type f -name '*.cpp')
OBJ= $(SRC:.cpp=.o)

all: $(OBJ)

%.o: %.cpp
	mkdir -p "../build/$(@D)"
	$(CC) $(CFLAGS) -o ../build/$@ $^
