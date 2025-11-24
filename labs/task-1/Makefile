CC := gcc
CFLAGS := -Wall -Wextra -Iinclude

SRCS := $(wildcard src/*.c)
BINS := $(patsubst src/%.c,bin/%,$(SRCS))

all: bin/main

bin/main: $(SRCS) | bin
	$(CC) $(CFLAGS) $(SRCS) -o $@

bin:
	mkdir -p bin

run: bin/main
	./$< $(ARGS)

clean:
	rm -rf bin