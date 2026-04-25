CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c11 -I include
NCURSES_FLAGS := -lncurses
RT_FLAGS := -lrt
THREAD_FLAGS := -pthread

all: bin/posix_server bin/posix_client bin/sysv_server bin/sysv_client bin/chat_server bin/chat_client

bin/posix_server: src/posix_server.c include/chat.h | bin
	$(CC) $(CFLAGS) $< $(RT_FLAGS) -o $@

bin/posix_client: src/posix_client.c include/chat.h | bin
	$(CC) $(CFLAGS) $< $(RT_FLAGS) -o $@

bin/sysv_server: src/sysv_server.c include/chat.h | bin
	$(CC) $(CFLAGS) $< -o $@

bin/sysv_client: src/sysv_client.c include/chat.h | bin
	$(CC) $(CFLAGS) $< -o $@

bin/chat_server: src/chat_server.c include/chat.h | bin
	$(CC) $(CFLAGS) $< $(THREAD_FLAGS) $(RT_FLAGS) -o $@

bin/chat_client: src/chat_client.c src/chat_ui.c include/chat.h include/chat_ui.h | bin
	$(CC) $(CFLAGS) src/chat_client.c src/chat_ui.c $(THREAD_FLAGS) $(NCURSES_FLAGS) $(RT_FLAGS) -o $@

bin:
	mkdir -p bin

run-posix-server: bin/posix_server
	./$<

run-posix-client: bin/posix_client
	./$<

run-sysv-server: bin/sysv_server
	./$<

run-sysv-client: bin/sysv_client
	./$<

run-chat-server: bin/chat_server
	./$<

run-chat-client: bin/chat_client
	./$<

run: run-chat-client

clean:
	rm -rf bin
