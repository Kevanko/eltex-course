#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

void read_from_pipe(int file)
{
  int c;
  FILE *stream = fdopen(file, "r");
  while ((c = fgetc(stream)) != EOF)
    putchar(c);
  fclose(stream);
}

void write_to_pipe(int file)
{
  FILE *stream = fdopen(file, "w");
  fprintf(stream, "Hi!\n");
  fclose(stream);
}

int main(void)
{
  int my_pipe[2]; /* 0 is read, 1 is write */
  pid_t pid;

  /* create a pipe */
  if (pipe(my_pipe) == -1)
  {
    perror("pipe failed");
    exit(EXIT_FAILURE);
  }

  /* fork a child process */
  pid = fork();

  if (pid == (pid_t)0)
  {
    /* this is the child process (only read) */
    close(my_pipe[1]); /* close the write */
    read_from_pipe(my_pipe[0]);
    return EXIT_SUCCESS;
  }
  else if (pid < (pid_t)0)
  {
    /* The fork failed. */
    fprintf(stderr, "Fork failed.\n");
    return EXIT_FAILURE;
  }
  else
  {
    /* this is the parent process (only write) */
    close(my_pipe[0]); /* close the read */
    write_to_pipe(my_pipe[1]);
    wait(NULL); /* wait for the child to finish */
    return EXIT_SUCCESS;
  }
}