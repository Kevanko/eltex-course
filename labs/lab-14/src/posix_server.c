#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <semaphore.h>
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

    shm_unlink(POSIX_EXCHANGE_SHM_NAME);

    fd = shm_open(POSIX_EXCHANGE_SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        return 1;
    }

    if (ftruncate(fd, sizeof(*memory)) == -1) {
        perror("ftruncate");
        close(fd);
        shm_unlink(POSIX_EXCHANGE_SHM_NAME);
        return 1;
    }

    memory = mmap(NULL, sizeof(*memory), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (memory == MAP_FAILED) {
        perror("mmap");
        close(fd);
        shm_unlink(POSIX_EXCHANGE_SHM_NAME);
        return 1;
    }

    memset(memory, 0, sizeof(*memory));
    if (sem_init(&memory->server_ready, 1, 0) == -1 ||
        sem_init(&memory->client_ready, 1, 0) == -1) {
        perror("sem_init");
        munmap(memory, sizeof(*memory));
        close(fd);
        shm_unlink(POSIX_EXCHANGE_SHM_NAME);
        return 1;
    }

    snprintf(memory->server_text, sizeof(memory->server_text), "Hi!");
    sem_post(&memory->client_ready);
    sem_wait(&memory->server_ready);

    printf("Client message: %s\n", memory->client_text);

    sem_destroy(&memory->server_ready);
    sem_destroy(&memory->client_ready);
    munmap(memory, sizeof(*memory));
    close(fd);
    shm_unlink(POSIX_EXCHANGE_SHM_NAME);

    return 0;
}
