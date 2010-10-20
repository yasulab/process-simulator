#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

static char buffer[BUFSIZ];

void copy(FILE *fin, FILE *fout)
{
  while (fgets(buffer, BUFSIZ, fin) != NULL) {
    fputs(buffer, fout);
    fflush(fout);
  }
}

void parentProcess(int wfd)
{
  FILE *fp = fdopen(wfd, "w");
  int status;

  if (fp == NULL) {
    perror("parent: fdopen");
    exit(3);
  }

  copy(stdin, fp);
  fclose(fp);

  if (wait(&status) == -1) {
    perror("wait");
    exit(4);
  }
}

void childProcess(int rfd)
{
  FILE *fp = fdopen(rfd, "r");

  if (fp == NULL) {
    perror("child: fdopen");
    _exit(3); /* C99 なら _Exit() でもよい。ふつうに exit() でもこの場合は違いはないでしょうけど^^ */
  }
  //copy(fp, stdout);
  while (fgets(buffer, BUFSIZ, fp) != NULL) {
    printf("buffer=%s",buffer);
    
    //fputs(buffer, stdout);
    if(!strcmp(buffer, "Q\n")){
      printf("End of one unit of time.\n");
    }else if(!strcmp(buffer, "U\n")){
      printf("Unblock the first simulated process in blocked queue.\n");
    }else if(!strcmp(buffer, "P\n")){
      printf("Print the current state of the system.\n");
    }else if(!strcmp(buffer, "T\n")){
      printf("Print the average turnaround time, and terminate the system.\n");
    }else{
      printf("Unknown command.\n");
    }
    fflush(stdout);
  }
  fclose(fp);
}

int main(void)
{
  int rv = 0, fd[2], pid;

  if (pipe(fd)) {
    perror("pipe");
    rv = 1;
  } else if ((pid = fork()) == -1) {
    perror("fork");
    rv = 2;
  } else if (pid > 0) {
    close(fd[0]);
    parentProcess(fd[1]);
  } else {
    close(fd[1]);
    childProcess(fd[0]);
  }

  return rv;
}
