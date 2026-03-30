#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

int main(void)
{
    /* remove existing pipe */
    unlink("mypipe");

    /* create a named pipe (FIFO) */
    if (mkfifo("mypipe", 0666) == -1)
    {
        perror("mkfifo");
        return 1;
    }
    /* open the pipe for writing */
    int fd = open("mypipe", O_WRONLY);
    if (fd == -1)
    {
        perror("open");
        return 1;
    }

    /* write a message to the pipe */
    const char *message = "Hi!";
    write(fd, message, strlen(message));

    /* close the pipe */
    close(fd);

    return 0;
}
