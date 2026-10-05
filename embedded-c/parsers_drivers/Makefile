CC = clang
CFLAGS = -Wall -Wextra -g

TARGETS = tlv_parser serialize camera_isp

.PHONY: all clean

all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(TARGETS) *.o *.bin *.dSYM
