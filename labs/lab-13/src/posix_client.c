#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>
#include <fcntl.h>

#define MAX_MSG_SIZE 128
#define SERVER_QUEUE "/POSIX-Server"
#define CLIENT_QUEUE "/POSIX-Client"

int main(void)
{
    mqd_t mq_client = mq_open(CLIENT_QUEUE, O_RDONLY);
    if (mq_client == (mqd_t)-1) {
        perror("mq_open /POSIX-Client");
        return 1;
    }

    mqd_t mq_server = mq_open(SERVER_QUEUE, O_WRONLY);
    if (mq_server == (mqd_t)-1) {
        perror("mq_open /POSIX-Server");
        mq_close(mq_client);
        return 1;
    }

    char reply[MAX_MSG_SIZE];

    if (mq_receive(mq_client, reply, sizeof(reply), NULL) == -1) {
        perror("mq_receive");
        mq_close(mq_client);
        mq_close(mq_server);
        return 1;
    }

    printf("Server message: %s\n", reply);

    char send_msg[MAX_MSG_SIZE];

    snprintf(send_msg, sizeof(send_msg), "Hello!");
    if (mq_send(mq_server, send_msg, strlen(send_msg) + 1, 0) == -1) {
        perror("mq_send");
        mq_close(mq_client);
        mq_close(mq_server);
        return 1;
    }

    mq_close(mq_client);
    mq_close(mq_server); 

    return 0;
}
