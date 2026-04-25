#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

#include "chat.h"

enum SysvSemaphoreIndex {
    SYSV_SEM_SERVER_READY = 0,
    SYSV_SEM_CLIENT_READY = 1
};

static int semaphore_wait(int semid, unsigned short sem_num)
{
    struct sembuf operation = {
        .sem_num = sem_num,
        .sem_op = -1,
        .sem_flg = 0
    };

    while (semop(semid, &operation, 1) == -1) {
        if (errno != EINTR) {
            return -1;
        }
    }

    return 0;
}

static int semaphore_post(int semid, unsigned short sem_num)
{
    struct sembuf operation = {
        .sem_num = sem_num,
        .sem_op = 1,
        .sem_flg = 0
    };

    return semop(semid, &operation, 1);
}

int main(void)
{
    int shmid;
    int semid;
    key_t key;
    key_t sem_key;
    SysvExchangeMemory *memory;

    key = ftok(SYSV_SHM_KEY_PATH, SYSV_EXCHANGE_KEY_ID);
    if (key == -1) {
        perror("ftok");
        return 1;
    }

    shmid = shmget(key, sizeof(*memory), 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    sem_key = ftok(SYSV_SHM_KEY_PATH, SYSV_SEM_KEY_ID);
    if (sem_key == -1) {
        perror("ftok");
        return 1;
    }

    semid = semget(sem_key, 2, 0666);
    if (semid == -1) {
        perror("semget");
        return 1;
    }

    memory = shmat(shmid, NULL, 0);
    if (memory == (void *)-1) {
        perror("shmat");
        return 1;
    }

    if (semaphore_wait(semid, SYSV_SEM_SERVER_READY) == -1) {
        perror("semop wait");
        shmdt(memory);
        return 1;
    }

    printf("Server message: %s\n", memory->server_text);

    snprintf(memory->client_text, sizeof(memory->client_text), "Hello!");
    if (semaphore_post(semid, SYSV_SEM_CLIENT_READY) == -1) {
        perror("semop post");
        shmdt(memory);
        return 1;
    }

    shmdt(memory);

    return 0;
}
