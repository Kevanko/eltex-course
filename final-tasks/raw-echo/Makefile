CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c11

all: bin/server bin/client

bin/server: src/server.c | bin
	$(CC) $(CFLAGS) $< -o $@

bin/client: src/client.c | bin
	$(CC) $(CFLAGS) $< -o $@

bin:
	mkdir -p bin

run-server: bin/server
	./$<

run-client: bin/client
	./$<

clean:
	rm -rf bin

