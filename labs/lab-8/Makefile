CC := gcc
CFLAGS := -Wall -Wextra -I include -Werror -fPIC

LIB_SRCS := src/add.c src/sub.c src/mul.c src/div.c
LIB_OBJS := $(patsubst src/%.c,bin/%.o,$(LIB_SRCS))
LIB := bin/libcalc.so

all: bin/main

# Обьектные файлы
bin/%.o: src/%.c | bin
	$(CC) $(CFLAGS) -c $< -o $@

# Библиотека (используем gcc -shared вместо ar)
$(LIB): $(LIB_OBJS)
	$(CC) -shared -o $@ $^

# Сборка
bin/main: src/main.c $(LIB) | bin
	$(CC) $(CFLAGS) $< -Lbin -lcalc -o $@

bin:
	mkdir -p bin

run: bin/main
	LD_LIBRARY_PATH=$(shell pwd)/bin ./$< $(ARGS)

clean:
	rm -rf bin
