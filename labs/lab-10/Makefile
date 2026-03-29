CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c11

all: bin/1 bin/2 bin/3

bin/1: src/1.c | bin
	$(CC) $(CFLAGS) $< -o $@

bin/2: src/2.c | bin
	$(CC) $(CFLAGS) $< -o $@

bin/3: src/3.c | bin
	$(CC) $(CFLAGS) $< -o $@

bin:
	mkdir -p bin

run-1: bin/1
	./$<

run-2: bin/2
	./$<

run-3: bin/3
	./$<

run: run-3

clean:
	rm -rf bin
