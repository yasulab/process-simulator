#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(void){
  pid_t result_pid;
  
  result_pid = fork();
  
  fprintf(stdout,"fork done\n");

  switch(result_pid){
  case 0:
    /* getpidは自分のPIDを返す. */
    /* getppiは自分のPPID(親のPID)を返す */
    fprintf(stdout,"child process.\tpid = %d.\tmy ppid = %d\n",getpid(),getppid());
    break;
  case -1:
    fprintf(stderr,"fork failed.\n");
    break;
  default:
    fprintf(stdout,"parent process.\tpid = %d.\tmy child's pid = %d\n",getpid(),result_pid);
    break;
  }

  return 0;
}

