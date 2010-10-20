#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void){
  pid_t result_pid;

  result_pid = fork();

  fprintf(stdout,"fork done\n");

  switch(result_pid){
  case 0:
    /* 3秒待つ */
    sleep(3);
    /* getpidは自分のPIDを返す. */
    /* getppiは自分のPPID(親のPID)を返す */
    fprintf(stdout,"child process.\tpid = %d.\tmy ppid = %d\n",getpid(),getppid());
    break;
  case -1:
    fprintf(stderr,"fork failed.\n");
    break;
  default:
    fprintf(stdout,"parent process.\tpid = %d.\tmy child's pid = %d\n",getpid(),result_pid);
    {
      int status;
      /* PIDを指定して待つ.三番目の引数はオプション */
      waitpid(result_pid,&status,WUNTRACED);
      fprintf(stdout,"child process done.");
      /* 子プロセスの終了ステータスを見る */
      if(WIFEXITED(status)){
	fprintf(stdout,"exit status = %d\n",WEXITSTATUS(status));
      }else{
	fprintf(stdout,"exit abnomally\n");
      }
    }
    break;
  }

  return 0;
}

