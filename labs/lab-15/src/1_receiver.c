#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void handle_sigusr1(int signo)
{
    (void)signo;
    printf("Received SIGUSR1\n");
    fflush(stdout);
}

int main(void)
{
    struct sigaction action;

    memset(&action, 0, sizeof(action));
    action.sa_handler = handle_sigusr1;
    sigemptyset(&action.sa_mask);

    if (sigaction(SIGUSR1, &action, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    printf("Receiver PID: %ld\n", (long)getpid());
    printf("Waiting for SIGUSR1...\n");

    while (1) {
        pause();
    }
}
