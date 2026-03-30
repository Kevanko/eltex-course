#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

static void parse_args(char *line, char *argv[])
{
    int argc = 0;
    char *token = strtok(line, " \t");

    while (token != NULL && argc < MAX_ARGS - 1) {
        argv[argc++] = token;
        token = strtok(NULL, " \t");
    }

    argv[argc] = NULL;
}

int main(void)
{
    char line[MAX_LINE];
    char *argv[MAX_ARGS];
    char *left_argv[MAX_ARGS];
    char *right_argv[MAX_ARGS];

    while (1) {
        printf("mini-bash> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            putchar('\n');
            break;
        }

        line[strcspn(line, "\n")] = '\0';

        if (line[0] == '\0') {
            continue;
        }

        if (strcmp(line, "exit") == 0) {
            break;
        }

        char *pipe_pos = strchr(line, '|');

        if (pipe_pos != NULL) {
            int fd[2];
            pid_t left_pid;
            pid_t right_pid;
            char *left = line;
            char *right;

            *pipe_pos = '\0';
            right = pipe_pos + 1;

            parse_args(left, left_argv);
            parse_args(right, right_argv);

            if (left_argv[0] == NULL || right_argv[0] == NULL) {
                continue;
            }

            if (pipe(fd) == -1) {
                perror("pipe");
                continue;
            }

            left_pid = fork();
            if (left_pid < 0) {
                perror("fork");
                close(fd[0]);
                close(fd[1]);
                continue;
            }

            if (left_pid == 0) {
                dup2(fd[1], STDOUT_FILENO);
                close(fd[0]);
                close(fd[1]);
                execvp(left_argv[0], left_argv);
                perror("execvp");
                exit(1);
            }

            right_pid = fork();
            if (right_pid < 0) {
                perror("fork");
                close(fd[0]);
                close(fd[1]);
                waitpid(left_pid, NULL, 0);
                continue;
            }

            if (right_pid == 0) {
                dup2(fd[0], STDIN_FILENO);
                close(fd[1]);
                close(fd[0]);
                execvp(right_argv[0], right_argv);
                perror("execvp");
                exit(1);
            }

            close(fd[0]);
            close(fd[1]);
            waitpid(left_pid, NULL, 0);
            waitpid(right_pid, NULL, 0);
            continue;
        }

        parse_args(line, argv);
        if (argv[0] == NULL) {
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            execvp(argv[0], argv);
            perror("execvp");
            exit(1);
        }

        int status = 0;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Exit status: %d\n", WEXITSTATUS(status));
        } else {
            printf("Process finished abnormally\n");
        }
    }

    return 0;
}
