CSRC= $(wildcard *.c)
COBJ= $(CSRC:.c=.o)

CPPSRC= $(wildcard *.cpp)
CPPOBJ= $(CPPSRC:.cpp=.o)

all: $(COBJ) $(CPPOBJ)

%.o: %.c
	$(CC) $(CFLAGS) -o ../build/$@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -o ../build/$@ $^
