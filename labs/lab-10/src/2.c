#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void print_process(const char *name)
{
    printf("%s: pid=%d, ppid=%d\n", name, getpid(), getppid());
    fflush(stdout);
}

static void wait_for_children(int count)
{
    for (int i = 0; i < count; ++i) {
        wait(NULL);
    }
}

int main(void)
{
    print_process("Parent");

    pid_t process1 = fork();
    if (process1 < 0) {
        perror("fork");
        return 1;
    }

    if (process1 == 0) {
        print_process("Process1");

        pid_t process3 = fork();
        if (process3 < 0) {
            perror("fork");
            exit(1);
        }

        if (process3 == 0) {
            print_process("Process3");
            exit(0);
        }

        pid_t process4 = fork();
        if (process4 < 0) {
            perror("fork");
            exit(1);
        }

        if (process4 == 0) {
            print_process("Process4");
            exit(0);
        }

        wait_for_children(2);
        exit(0);
    }

    pid_t process2 = fork();
    if (process2 < 0) {
        perror("fork");
        return 1;
    }

    if (process2 == 0) {
        print_process("Process2");

        pid_t process5 = fork();
        if (process5 < 0) {
            perror("fork");
            exit(1);
        }

        if (process5 == 0) {
            print_process("Process5");
            exit(0);
        }

        wait_for_children(1);
        exit(0);
    }

    wait_for_children(2);
    return 0;
}
