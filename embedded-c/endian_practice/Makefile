CC ?= gcc
CFLAGS ?= -Wall -Wextra -Werror -pedantic -std=c11 -Iinclude -O2 -g
SANFLAGS ?= -fsanitize=address,undefined

TARGET = test_runner
SRCS = src/endian_swap.c src/main.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(SANFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) $(SANFLAGS) -c $< -o $@

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all test clean