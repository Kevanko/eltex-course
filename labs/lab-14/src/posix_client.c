#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "chat.h"

int main(void)
{
    int fd;
    PosixExchangeMemory *memory;

    fd = shm_open(POSIX_EXCHANGE_SHM_NAME, O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        return 1;
    }

    memory = mmap(NULL, sizeof(*memory), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (memory == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    sem_wait(&memory->client_ready);
    printf("Server message: %s\n", memory->server_text);

    snprintf(memory->client_text, sizeof(memory->client_text), "Hello!");
    sem_post(&memory->server_ready);

    munmap(memory, sizeof(*memory));
    close(fd);

    return 0;
}
