CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c11

all: bin/taxi-cli

bin/taxi-cli: src/main.c | bin
	$(CC) $(CFLAGS) $< -o $@

bin:
	mkdir -p bin

run: bin/taxi-cli
	./$<

clean:
	rm -rf bin
