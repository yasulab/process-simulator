#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_STR 256 /* 文字列入力用配列長 */
#define MAX_LINE 30
#define MAX_TABLE 30

#define BLOCK 0
#define READY 1
#define RUNNING 2

struct Cpu
{
  //int pointer;
  int pc;
  int pid;
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

struct State{
  int pid;
};

static char buffer[BUFSIZ];

typedef struct Que {
  struct Que *next;
  struct State *state;
}QUE;
//static QUE *First;

void *get_que(QUE *First){
  QUE *lp = First;
  struct State *state;
  if(lp==NULL){    /* データがキューにない */
    return NULL;
  }
  state = lp->state;
  lp = lp->next;
  free(First);
  First=lp;
  return state;
}

void add_que(QUE *First, struct State *state){
  QUE *lp=First;
  if(lp!=NULL){
    while(lp->next!=NULL){
      lp=lp->next;
    }
    lp = lp->next = (QUE*)malloc(sizeof(QUE));
  }else{
    lp = First=(QUE*)malloc(sizeof(QUE));
  }
  lp->state = state;
  lp->next = NULL;
  return;
}

void contextSwitch(struct Cpu *cpu, struct PcbTable *ptable){
  struct Cpu temp;
  temp.pc = cpu->pc;
  temp.pid = cpu->pid;
  temp.value = cpu->value;
  temp.t_slice = cpu->t_slice;
  temp.t_remain = cpu->t_remain;

  cpu->pc =  ptable->pc;
  cpu->pid = ptable->pid;
  cpu->value = ptable->value;
  cpu->t_slice = ptable->t_start; // ???
  cpu->t_remain = ptable->t_cpu; // ???

  ptable->pc = temp.pc;
  ptable->pid = temp.pid;
  ptable->ppid = 0; // ???
  ptable->value = temp.value;
  ptable->priority = 0; // ???
  ptable->state = READY; // ???
  ptable->t_start = temp.t_slice; // ???
  ptable->t_cpu = temp.t_remain; // ???
}

int readProgram(char *fname, char prog[][MAX_STR]){
  FILE *fp;          /* ファイルポインタ用 */
  char buff[MAX_STR], *pp;    /* 文字列用 */
  fp = fopen(fname, "r");    /* ファイルオープン */
  if(fp == NULL){            /* オープン失敗 */
    printf("ファイルがオープンできません\n");
    exit(1);               /* 強制終了 */
  }

  int i=0;
  while(1){    /* 永久ループ */
    pp = fgets(buff, MAX_STR, fp);    /* 1行読み込み */
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
  char cmd[MAX_STR];
  
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

void processManagerProcess(int rfd)
{
  FILE *fp = fdopen(rfd, "r");
  char *fname = "init";
  char prog[MAX_LINE][MAX_STR];
  char **cmd;
  int i=0;
  int pid_cnt = 0;
  int arg;
  int x,y; // iterators

  /* ProcessManager's 6 Data Structures*/
  int time;
  struct Cpu cpu;
  struct PcbTable ptable[MAX_TABLE];
  QUE ready_state;
  QUE blocked_state;
  struct State running_state;
  /**************************************/

  /* Initializing */
  readProgram(fname, prog);
  time = 0;
  cpu.pc = 0;
  cpu.value = 0;
  ptable[0].pid = pid_cnt;
  running_state.pid = 0;
  /****************/

  pid_cnt++;

  int n;
  while (fgets(buffer, BUFSIZ, fp) != NULL) {
    printf("buffer=%s",buffer);
    if(!strcmp(buffer, "Q\n")){
      printf("End of one unit of time.\n");
      cmd = split(&n, prog[cpu.pc]);
      printf("cmd[0]=%s\n",cmd[0]);
      
      if(!strcmp(cmd[0],"S")){
	printf("Set the value of the integer variable to %d.\n", atoi(cmd[1]));
	cpu.value = atoi(cmd[1]);
      }else if(!strcmp(cmd[0],"A")){
	printf("Add %d to the value of the integer variable.\n", atoi(cmd[1]));
	cpu.value += atoi(cmd[1]);
      }else if(!strcmp(cmd[0],"D")){
	printf("Substract %d from the value of the integer variable.\n", atoi(cmd[1]));
	cpu.value -= atoi(cmd[1]);
      }else if(!strcmp(cmd[0],"B")){
	printf("Block this simulated process.\n");
      }else if(!strcmp(cmd[0],"E\n")){
	printf("Terminate this simulated process.\n");
      }else if(!strcmp(cmd[0],"F")){
	printf("Create a new simulated process at %d times.\n", atoi(cmd[1]));
	arg = atoi(cmd[1]);
	for(x=0; x<arg; x++){
	  // create new processes.
	  printf("Created a process with pid=%d.\n", pid_cnt);
	  pid_cnt++;
	}
	
      }else if(!strcmp(cmd[0],"R")){
	printf("Replace the program of the simulated process with the program in the file '%s'.\n", cmd[1]);
	cpu.pc = 0;
	for(x=0;x<MAX_LINE;x++){
	  for(y=0;y<MAX_STR;y++){
	    prog[x][y] = '\0';
	  }
	}
	readProgram(cmd[1], prog);
	
      }else{
	printf("Unknown Instruction.");	
      }
      free(cmd);
      time++;
      cpu.pc++;
      
    }else if(!strcmp(buffer, "U\n")){
      printf("Unblock the first simulated process in blocked queue.\n");
      if(get_que(&blocked_state) == NULL){
	printf("There are no processes in BlockedState.");
      }else{
	// unblock the process.
	add_que(&ready_state, get_que(&blocked_state));
      }

    }else if(!strcmp(buffer, "P\n")){
      printf("Print the current state of the system.\n");
    }else if(!strcmp(buffer, "T\n")){
      printf("Print the average turnaround time, and terminate the system.\n");
      //exit(1);
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

