CC      = clang
CFLAGS  = -Wall -Wextra -g

.PHONY: all test clean run

all: simpleshell donothing

simpleshell: simpleshell.c
	$(CC) $(CFLAGS) simpleshell.c -o simpleshell

donothing: donothing.c
	$(CC) $(CFLAGS) donothing.c -o donothing

test: all
	@echo "=== Running simpleshell smoke test ==="
	@printf "donothing\nexit\n" | ./simpleshell > /dev/null 2>&1 || exit 1
	@echo "simpleshell built and passed smoke test!"

run: simpleshell
	./simpleshell

clean:
	rm -rf simpleshell donothing *.o *.dSYM
