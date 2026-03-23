CC := gcc
CFLAGS := -Wall -Wextra -std=c11
STRICT_CFLAGS := -Werror
MY_MC_SRCS := src/my_mc_main.c src/my_mc_fs.c src/my_mc_ui.c src/my_mc_input.c

all: bin/part1 bin/my_mc

bin/part1: src/part1.c | bin
	$(CC) $(CFLAGS) $< -o $@

bin/my_mc: $(MY_MC_SRCS) include/my_mc.h | bin
	$(CC) $(CFLAGS) $(STRICT_CFLAGS) -I include $(MY_MC_SRCS) -lncurses -o $@

bin:
	mkdir -p bin

run-part1: bin/part1
	./$<

run-my_mc: bin/my_mc
	./$<

run: run-my_mc

clean:
	rm -rf bin
