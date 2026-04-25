#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_MSG_SIZE 128
#define SERVER_QUEUE "/POSIX-Server"
#define CLIENT_QUEUE "/POSIX-Client"

int main(void)
{
    struct mq_attr attr = {
        .mq_flags = 0,
        .mq_maxmsg = 10,
        .mq_msgsize = MAX_MSG_SIZE,
        .mq_curmsgs = 0
    };

    if (mq_unlink(CLIENT_QUEUE) == -1 && errno != ENOENT) {
        perror("mq_unlink /POSIX-Client");
        return 1;
    }

    if (mq_unlink(SERVER_QUEUE) == -1 && errno != ENOENT) {
        perror("mq_unlink /POSIX-Server");
        return 1;
    }

    mqd_t mq_client = mq_open(CLIENT_QUEUE, O_WRONLY | O_CREAT, 0666, &attr);
    if (mq_client == (mqd_t)-1) {
        perror("mq_open /POSIX-Client");
        return 1;
    }

    mqd_t mq_server = mq_open(SERVER_QUEUE, O_RDONLY | O_CREAT, 0666, &attr);
    if (mq_server == (mqd_t)-1) {
        perror("mq_open /POSIX-Server");
        mq_close(mq_client);
        mq_unlink(CLIENT_QUEUE);
        return 1;
    }

    char send_msg[MAX_MSG_SIZE];
    snprintf(send_msg, sizeof(send_msg), "Hi!");
    if (mq_send(mq_client, send_msg, strlen(send_msg) + 1, 0) == -1) {
        perror("mq_send");
        mq_close(mq_client);
        mq_close(mq_server);
        return 1;
    }

    char reply[MAX_MSG_SIZE];
    if (mq_receive(mq_server, reply, sizeof(reply), NULL) == -1) {
        perror("mq_receive");
        mq_close(mq_client);
        mq_close(mq_server);
        return 1;
    }
    printf("Client message: %s\n", reply);

    mq_close(mq_client);
    mq_close(mq_server);

    mq_unlink(CLIENT_QUEUE);
    mq_unlink(SERVER_QUEUE);
    
    return 0;
}
