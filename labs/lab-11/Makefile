CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c11 -pthread

all: bin/main

bin/main: src/main.c | bin
	$(CC) $(CFLAGS) $< -o $@

bin:
	mkdir -p bin

run: bin/main
	./$<

clean:
	rm -rf bin
