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

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
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
    union semun arg = {0};

    key = ftok(SYSV_SHM_KEY_PATH, SYSV_EXCHANGE_KEY_ID);
    if (key == -1) {
        perror("ftok");
        return 1;
    }

    shmid = shmget(key, sizeof(*memory), 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    sem_key = ftok(SYSV_SHM_KEY_PATH, SYSV_SEM_KEY_ID);
    if (sem_key == -1) {
        perror("ftok");
        shmctl(shmid, IPC_RMID, NULL);
        return 1;
    }

    semid = semget(sem_key, 2, 0666 | IPC_CREAT);
    if (semid == -1) {
        perror("semget");
        shmctl(shmid, IPC_RMID, NULL);
        return 1;
    }

    arg.val = 0;
    if (semctl(semid, SYSV_SEM_SERVER_READY, SETVAL, arg) == -1 ||
        semctl(semid, SYSV_SEM_CLIENT_READY, SETVAL, arg) == -1) {
        perror("semctl");
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);
        return 1;
    }

    memory = shmat(shmid, NULL, 0);
    if (memory == (void *)-1) {
        perror("shmat");
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);
        return 1;
    }

    memset(memory, 0, sizeof(*memory));
    snprintf(memory->server_text, sizeof(memory->server_text), "Hi!");
    if (semaphore_post(semid, SYSV_SEM_SERVER_READY) == -1) {
        perror("semop post");
        shmdt(memory);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);
        return 1;
    }

    if (semaphore_wait(semid, SYSV_SEM_CLIENT_READY) == -1) {
        perror("semop wait");
        shmdt(memory);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);
        return 1;
    }

    printf("Client message: %s\n", memory->client_text);

    shmdt(memory);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);

    return 0;
}
