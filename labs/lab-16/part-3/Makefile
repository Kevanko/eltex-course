CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c11

TASKS := 1 2
BINS := $(foreach task,$(TASKS),$(task)/bin/server $(task)/bin/client)

.PRECIOUS: %/bin

all: $(BINS)

%/bin/server: %/server.c | %/bin
	$(CC) $(CFLAGS) $< -o $@

%/bin/client: %/client.c | %/bin
	$(CC) $(CFLAGS) $< -o $@

%/bin:
	mkdir -p $@

run-1-server: 1/bin/server
	./$<

run-1-client: 1/bin/client
	./$<

run-2-server: 2/bin/server
	./$<

run-2-client: 2/bin/client
	./$<

clean:
	rm -rf 1/bin 2/bin
