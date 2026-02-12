CC := gcc
CFLAGS := -Wall -Wextra -I include -Werror

LIB_SRCS := src/add.c src/sub.c src/mul.c src/div.c
LIB_OBJS := $(patsubst src/%.c,bin/%.o,$(LIB_SRCS))
LIB := bin/libcalc.a

all: bin/main

# Обьектные файлы
bin/%.o: src/%.c | bin
	$(CC) $(CFLAGS) -c $< -o $@

# Библиотека
$(LIB): $(LIB_OBJS)
	ar rcs $@ $^

# Сборка
bin/main: src/main.c $(LIB) | bin
	$(CC) $(CFLAGS) $< -Lbin -lcalc -o $@

bin:
	mkdir -p bin

run: bin/main
	./$< $(ARGS)

clean:
	rm -rf bin
