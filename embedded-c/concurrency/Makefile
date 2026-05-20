.PHONY: all clean run solutions

CC = gcc
CFLAGS = -Wall -g -pthread -c
LDFLAGS = -pthread
SOURCES = basicMutex1.c basicMutex2.c
PROG1 = basicMutex1
PROG2 = basicMutex2

# Default target builds both binaries
all: condwait.bin $(PROG1) $(PROG2) barrier.bin producerconsumer.bin

# Pattern rule to link object files into binaries with pthread support
%.bin: %.o
	gcc -o $@ $^ -pthread

$(PROG1): basicMutex1.o
	$(CC) $(LDFLAGS) $^ -o $@

$(PROG2): basicMutex2.o
	$(CC) $(LDFLAGS) $^ -o $@

# Implicit rule for %.c to %.o is automatically handled by Make, 
# but you can define CFLAGS at the top if you need specific compiler flags (e.g., CFLAGS = -Wall -g)

# Target to specifically compile and run condwait.bin
run: condwait.bin
	./condwait.bin

mutex1: basicMutex1.bin
	./basicMutex1.bin

mutex2: basicMutex2.bin
	./basicMutex2.bin

barrier: barrier.bin
	./barrier.bin

prodcons: producerconsumer.bin
	./producerconsumer.bin

# Target for solution binaries
solutions: condwait.bin basicMutex1.bin basicMutex2.bin barrier.bin 

clean:
	rm -f *.o *.bin *~ $(PROG1) $(PROG2)