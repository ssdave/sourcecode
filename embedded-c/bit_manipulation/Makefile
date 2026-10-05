CC = clang
CFLAGS = -Wall -Wextra -g

TARGETS = bit_basics bit_level2 registers

.PHONY: all clean

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(TARGETS) *.o *.bin *.dSYM
