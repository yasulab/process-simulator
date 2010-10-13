#include <stdio.h>

int pipe_fd[2];

void do_parent()
{
  int  status;
  char *p="Hello, my kid.";

  printf("this is parent.\n");

  close(pipe_fd[0]);
  while(*p){
    if(write(pipe_fd[1],p,1) < 0){
      perror("write");
      exit(1);
    }
    p++;
  }
  // 通信の終了を子プロセスに通知
  //  --> なければ read が待ち状態のままとなる
  close(pipe_fd[1]);

  if(wait(&status) < 0){
    perror("wait");
    exit(1);
  }
}

void do_child()
{
  int i,c;

  printf("this is child.\n");

  close(pipe_fd[1]);
  while((i=read(pipe_fd[0],&c,1)) > 0){
    putchar(c);
  }
  putchar('\n');
  close(pipe_fd[0]);
}

int main()
{
  int child;

  if(pipe(pipe_fd) < 0){
    perror("pipe");
    exit(1);
  }

  if((child = fork()) < 0){
    perror("fork");
    exit(1);
  }

  if(child) do_parent();
  else      do_child();

  return 0;

}
