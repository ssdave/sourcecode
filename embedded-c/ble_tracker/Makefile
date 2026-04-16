CC = clang
CFLAGS = -Wall -Wextra -g -pthread

.PHONY: all clean

all: problem1.bin problem2.bin problem3.bin problem4.bin

problem1.bin: Problem1\ -\ BLE\ Payload\ Bit\ Extraction.c
	$(CC) $(CFLAGS) "Problem1 - BLE Payload Bit Extraction.c" -o $@

problem2.bin: Problem2\ -\ The\ Interrupt-Safe\ Ring\ Buffer.c
	$(CC) $(CFLAGS) "Problem2 - The Interrupt-Safe Ring Buffer.c" -o $@

problem3.bin: Problem\ 3\ -\ The\ Anti-Stalking\ Debouncer.c
	$(CC) $(CFLAGS) "Problem 3 - The Anti-Stalking Debouncer.c" -o $@

problem4.bin: Problem\ 4\ -\ The\ Secure\ Broadcast\ Coordinator.c
	$(CC) $(CFLAGS) "Problem 4 - The Secure Broadcast Coordinator.c" -o $@

clean:
	rm -rf *.bin *.o *.dSYM myapp \
		"Problem1 - BLE Payload Bit Extraction" \
		"Problem2 - The Interrupt-Safe Ring Buffer" \
		"Problem 3 - The Anti-Stalking Debouncer" \
		"Problem 4 - The Secure Broadcast Coordinator"
