CC = clang
CFLAGS = -Wall -Wextra -g

TARGETS = ringbuffer ring_buffer movingaverage

.PHONY: all clean

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(TARGETS) *.o *.bin *.dSYM
