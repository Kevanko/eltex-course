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
    int qid = msgget(key, 0666 | IPC_CREAT);
    if(qid == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    struct msgbuf msg;
    msg.mtype = 1;
    msg.pid = getpid();
    snprintf(msg.text, sizeof(msg.text), "Hi!");
    if (msgsnd(qid, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        perror("msgsnd");
        msgctl(qid, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
        return 1;
    }

    struct msgbuf reply;
    if (msgrcv(qid, &reply, sizeof(reply) - sizeof(long), msg.pid, 0) == -1) {
        perror("msgrcv");
        msgctl(qid, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
        return 1;
    }
    printf("Client message: %s\n", reply.text);

    if (msgctl(qid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(EXIT_FAILURE);
    }
    
    return 0;
}
