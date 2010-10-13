#include <stdio.h>
#include <string.h>
#define MAX_INPUT 100

int pipe_fd[2];

void do_parent()
{
  int  status;
  char p[MAX_INPUT];
  char input[MAX_INPUT];

  //strcpy(p, "Hello, my kid.\0");
  printf("this is parent.\n");

  close(pipe_fd[0]);
  
  //printf( "Input: " );
  //scanf("%s", p);
  
  while(1){
    //gets(p);
    //printf("p=%s",p);
    //sleep(100);
    //*p = getchar();
    //*p = "Hello Pipe";
    while(*p){
      if(write(pipe_fd[1],p,1) < 0){
	perror("write");
	//exit(1);
      }
      p++;
    }
    //printf("Input: ");
    //scanf("%s", p);
    //p++;
    //*p = *(p+2);
  }
  // 通信の終了を子プロセスに通知
  //  --> なければ read が待ち状態のままとなる
  close(pipe_fd[1]);
  
  if(wait(&status) < 0){
    perror("wait");
    //exit(1);
  }
}

void do_child()
{
  int i,c;
  char recv[MAX_INPUT];
  //printf("this is child.\n");

  close(pipe_fd[1]);
  while(1){
    i=0;
    while((read(pipe_fd[0],&c,1)) > 0){
      printf("i=%d\n",i);
      recv[i] = (char)c;
      i++;
      //if((char)c == '\n') break;
    }
    recv[i] = '\0';
    printf("recv = %s", recv);
    
    if(!strcmp(recv, "Q\n")){
      printf("End of one unit of time.");
    }else if(!strcmp(recv, "U\n")){
      printf("Unblock the first simulated process in blocked queue.");
    }else if(!strcmp(recv, "P\n")){
      printf("Print the current state of the system.");
    }else if(!strcmp(recv, "T\n")){
      printf("Print the average turnaround time, and terminate the system.");
    }else{
      printf("Unknown command.");
    }
    printf("command done.");
  }
  close(pipe_fd[0]);
}

int main()
{
  int child;

  if(pipe(pipe_fd) < 0){
    perror("pipe");
    //exit(1);
  }

  if((child = fork()) < 0){
    perror("fork");
    //exit(1);
  }

  if(child) do_parent();
  else      do_child();

  return 0;

}
