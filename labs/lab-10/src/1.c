#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        printf("Child: pid=%d, ppid=%d\n", getpid(), getppid());
        return 5;
    }

    printf("Parent: pid=%d, ppid=%d\n", getpid(), getppid());

    int status = 0;
    wait(&status);

    if (WIFEXITED(status)) {
        printf("Child exit status: %d\n", WEXITSTATUS(status));
    } else {
        printf("Child finished abnormally\n");
    }

    return 0;
}
