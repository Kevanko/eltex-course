#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

void sig_handler(int sig_num, siginfo_t *info, void *args) {
    int *param = (int*)args;
    printf("Signal SIGUSR1! - %d %d %d\n", sig_num, *param, info->si_signo);
}

int main(){
    struct sigaction handler;
    sigset_t set;
    int ret;

    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    handler.sa_mask = set;
    handler.sa_sigaction =sig_handler;

    ret = sigaction(SIGUSR1, &handler, NULL);
    if(ret < 0) {
        perror("Can't set signal handler!");
        exit(EXIT_FAILURE);
    }
    printf("Waiting for signal... PID: %ld\n", (long)getpid());
    while(1) {
        pause();
    }
    exit(EXIT_SUCCESS);
}