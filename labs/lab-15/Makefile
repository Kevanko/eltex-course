CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c11

all: bin/1_receiver bin/1_sender bin/2 bin/3

bin/1_receiver: src/1_receiver.c | bin
	$(CC) $(CFLAGS) $< -o $@

bin/1_sender: src/1_sender.c | bin
	$(CC) $(CFLAGS) $< -o $@

bin/2: src/2.c | bin
	$(CC) $(CFLAGS) $< -o $@

bin/3: src/3.c | bin
	$(CC) $(CFLAGS) $< -o $@

bin:
	mkdir -p bin

run-1-receiver: bin/1_receiver
	./$<

run-1-sender: bin/1_sender
	@if [ -z "$(PID)" ]; then \
		echo "Usage: make run-1-sender PID=<pid>"; \
		exit 1; \
	fi
	./$< $(PID)

run-2: bin/2
	./$<

run-3: bin/3
	./$<

clean:
	rm -rf bin
