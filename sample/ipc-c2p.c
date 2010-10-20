#include <stdio.h>

int pipe_fd[2];

void do_parent()
{
  int i,c,status;

  printf("this is parent.\n");

  close(pipe_fd[1]);
  while((i=read(pipe_fd[0],&c,1)) > 0){
    putchar(c);
  }
  putchar('\n');
  close(pipe_fd[0]);

  if(wait(&status) < 0){
    perror("wait");
    exit(1);
  }
}

void do_child()
{
  char *p="Hello, my parent.";

  printf("this is child.\n");

  close(pipe_fd[0]);
  while(*p){
    if(write(pipe_fd[1],p,1) < 0){
      perror("write");
      exit(1);
    }
    p++;
  }

  // 通信の終了を子プロセスに通知
  close(pipe_fd[1]);
  // →子プロセスはプロセスの終了とともにディスクリプタを
  // 閉じるためなくてもよい
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
