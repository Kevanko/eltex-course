#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGINT);

    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
        perror("sigprocmask");
        return 1;
    }

    printf("SIGINT blocker PID: %ld\n", (long)getpid());
    printf("SIGINT is blocked. Waiting...\n");

    while (1) {
        pause();
    }
}
