#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct msgbuf {
    long mtype;
    pid_t pid;
    char text[128];
};

int main(void)
{
    key_t key = ftok("SystemV", 1);
    int qid = msgget(key, 0666);
    if (qid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    struct msgbuf reply;
    if (msgrcv(qid, &reply, sizeof(reply) - sizeof(long), 1, 0) == -1) {
        perror("msgrcv");
        return 1;
    }

    printf("Server message: %s\n", reply.text);

    struct msgbuf msg;
    msg.mtype = reply.pid;
    msg.pid = getpid();

    snprintf(msg.text, sizeof(msg.text), "Hello!");
    if (msgsnd(qid, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        return 1;
    }

    return 0;
}
