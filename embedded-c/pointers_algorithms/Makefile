CC = clang
CFLAGS = -Wall -Wextra -g

TARGETS = double_pointer remove_char binarysearch Hello

.PHONY: all clean

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(TARGETS) *.o *.bin *.dSYM
