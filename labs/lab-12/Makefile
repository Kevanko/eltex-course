CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c11

all: bin/1 bin/server bin/client bin/3

bin/1: src/1.c | bin
	$(CC) $(CFLAGS) $< -o $@

bin/server: src/server.c | bin
	$(CC) $(CFLAGS) $< -o $@

bin/client: src/client.c | bin
	$(CC) $(CFLAGS) $< -o $@

bin/3: src/3.c | bin
	$(CC) $(CFLAGS) $< -o $@

bin:
	mkdir -p bin

run-1: bin/1
	./$<

run-server: bin/server
	./$<

run-client: bin/client
	./$<

run-3: bin/3
	./$<

run: run-3

clean:
	rm -rf bin
