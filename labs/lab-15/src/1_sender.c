#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    long pid;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
        return 1;
    }

    pid = strtol(argv[1], NULL, 10);
    if (pid <= 0) {
        fprintf(stderr, "Invalid pid\n");
        return 1;
    }

    if (kill((pid_t)pid, SIGUSR1) == -1) {
        perror("kill");
        return 1;
    }

    printf("SIGUSR1 sent to PID %ld\n", pid);
    return 0;
}
