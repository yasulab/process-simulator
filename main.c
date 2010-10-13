#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define STR_MAX 256 /* 文字列入力用配列長 */
#define LINE_MAX 30

struct Cpu
{
  int pointer;
  int pc;
  int value;
  int t_slice;
  int t_remain;
};

struct PcbTable
{
  int pid;
  int ppid;
  int pc;
  int value;
  int priority;
  int state;
  int t_start;
  int t_cpu;
};

struct ReadyState{
  int *next;
  int pid[];
}ready_state;

struct BlockedState{
  int *next;
  int pid[];
}blocked_state;

struct RunningState{
  int pid;
}running_state;
  

static char buffer[BUFSIZ];

int readProgram(char *fname, char prog[][STR_MAX]){
  FILE *fp;          /* ファイルポインタ用 */
  char buff[STR_MAX], *pp;    /* 文字列用 */
  fp = fopen(fname, "r");    /* ファイルオープン */
  if(fp == NULL){            /* オープン失敗 */
    printf("ファイルがオープンできません\n");
    exit(1);               /* 強制終了 */
  }

  int i=0;
  while(1){    /* 永久ループ */
    pp = fgets(buff, STR_MAX, fp);    /* 1行読み込み */
    strcpy(prog[i], buff);
    if(pp == NULL){    /* 読み込み終了 */
      break;           /* ループ脱出 */
    }
    //printf("%s", buff);    /* 1行表示 */
    i++;
  }

  fclose(fp);    /* ファイルクローズ */
  return(0);
}

char **split(int *n, char *string)
{
  char **array=NULL;
  char *p=string;
  char *s;

  for(*n=0; (s = strtok(p, " ")) != NULL; (*n)++) {
    array = (char**)realloc(array, sizeof(char*) * (*n+1));
    array[*n] = s;
    p = NULL;
  }

  return array;
}

void copy(FILE *fin, FILE *fout)
{
  while (fgets(buffer, BUFSIZ, fin) != NULL) {
    fputs(buffer, fout);
    fflush(fout);
  }
}

void commanderProcess(int wfd)
{
  FILE *fp = fdopen(wfd, "w");
  int status;
  char cmd[STR_MAX];
  
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

void simulatedProcess(int wfd, int pid, char prog[][STR_MAX]){
  int i=0, j, n;
  char **cmd;
  while(strcmp(prog[i],"E\n")){
    //printf("prog[%d]=%s", i, prog[i]);
    cmd = split(&n, prog[i]);
    
    //printf("CMD=%s\tARGS=%s", cmd[0], cmd[1]);
    if(!strcmp(cmd[0],"S")){
      printf("Set the value of the integer variable to %d.\n", atoi(cmd[1]));
    }else if(!strcmp(cmd[0],"A")){
      printf("Add %d to the value of the integer variable.\n", atoi(cmd[1]));
    }else if(!strcmp(cmd[0],"D")){
      printf("Substract %d from the value of the integer variable.\n", atoi(cmd[1]));
    }else if(!strcmp(cmd[0],"B")){
      printf("Block this simulated process.\n");
    }else if(!strcmp(cmd[0],"E")){
      printf("Terminate this simulated process.\n");
    }else if(!strcmp(cmd[0],"F")){
      printf("Create a new simulated process at %d times.\n", atoi(cmd[1]));
    }else if(!strcmp(cmd[0],"R")){
      printf("Replace the program of the simulated process with the program in the file '%s'.\n", cmd[1]);
      int pc = 0;
    }else{
      printf("Unknown Instruction.");
    }
    free(cmd);
    i++;
  }
  printf("prog[%d]=%s", i, prog[i]);

  //i=0;
  
  printf("*** pid=%d was exit. ***\n", pid);
}


void processManagerProcess(int rfd)
{
  FILE *fp = fdopen(rfd, "r");
  char *fname = "init";
  char prog[LINE_MAX][STR_MAX];
  int i=0;

  int rv = 0, fd[2], pid;
  
  if (fp == NULL) {
    perror("child: fdopen");
    exit(3); 
  }
  //copy(fp, stdout);

  if (pipe(fd)) {
    perror("pipe");
    rv = 1;
  } else if ((pid = fork()) == -1) {
    perror("fork");
    rv = 2;
  } else if (pid > 0) {
    close(fd[0]);
    readProgram(fname, prog);
    simulatedProcess(fd[1], 0, prog);
  } else {
    close(fd[1]);
  }
  
  while (fgets(buffer, BUFSIZ, fp) != NULL) {
    printf("buffer=%s",buffer);
    if(!strcmp(buffer, "Q\n")){
      printf("End of one unit of time.\n");
    }else if(!strcmp(buffer, "U\n")){
      printf("Unblock the first simulated process in blocked queue.\n");
    }else if(!strcmp(buffer, "P\n")){
      printf("Print the current state of the system.\n");
    }else if(!strcmp(buffer, "T\n")){
      printf("Print the average turnaround time, and terminate the system.\n");
      exit(1);
    }else{
      printf("Unknown command.\n");
    }
    //fputs(buffer, stdout);
    
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
    commanderProcess(fd[1]);
  } else {
    close(fd[1]);
    processManagerProcess(fd[0]);
  }

  return rv;
}

