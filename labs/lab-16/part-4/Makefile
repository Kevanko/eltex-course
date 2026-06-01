CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c11

all: bin/server 1/bin/client 2/bin/client 3/bin/client

bin/server: server.c | bin
	$(CC) $(CFLAGS) $< -o $@

1/bin/client: 1/client.c | 1/bin
	$(CC) $(CFLAGS) $< -o $@

2/bin/client: 2/client.c | 2/bin
	$(CC) $(CFLAGS) $< -o $@

3/bin/client: 3/client.c | 3/bin
	$(CC) $(CFLAGS) $< -o $@

bin 1/bin 2/bin 3/bin:
	mkdir -p $@

run-server: bin/server
	./$<

run-1-client: 1/bin/client
	./$<

run-2-client: 2/bin/client
	./$<

run-3-client: 3/bin/client
	./$<

clean:
	rm -rf bin 1/bin 2/bin 3/bin

