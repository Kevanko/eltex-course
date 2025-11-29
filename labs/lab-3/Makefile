CC := gcc
CFLAGS := -Wall -Wextra -I include

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