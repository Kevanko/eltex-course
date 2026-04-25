#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    int signo;
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);

    if (sigprocmask(SIG_BLOCK, &set, NULL) == -1) {
        perror("sigprocmask");
        return 1;
    }

    printf("sigwait loop PID: %ld\n", (long)getpid());
    printf("Waiting for SIGUSR1...\n");

    while (1) {
        if (sigwait(&set, &signo) != 0) {
            perror("sigwait");
            return 1;
        }

        if (signo == SIGUSR1) {
            printf("Received SIGUSR1\n");
            fflush(stdout);
        }
    }
}
