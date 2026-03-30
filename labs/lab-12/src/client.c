#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main(void)
{
    /* open the pipe for reading */
    int fd = open("mypipe", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    /* read a message from the pipe */
    char buffer[100];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read == -1) {
        perror("read");
        close(fd);
        return 1;
    }
    buffer[bytes_read] = '\0';

    /* print the received message */
    printf("Received message: %s\n", buffer);

    /* close the pipe */
    close(fd);

    /* remove pipe */
    unlink("mypipe");

    return 0;
}
