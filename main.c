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
  struct State state;
  struct Que *next; /* pointer to next element in list */
} QUE;

/* Insert into a head */
QUE *insert_head(QUE **p, struct State s)
{
  QUE *n = (QUE *) malloc(sizeof(QUE));
  if (n == NULL) return NULL;

  n->next = *p;
  *p = n;
  n->state = s;

  return n;
}

/* Enque into a tail */
QUE *enqueue(QUE **p, struct State s)
{
  QUE *tail = *p;
  //printf("pid=%d\n", s.pid);
  if(*p == NULL) // For initial setup
    return insert_head(p, s);

  while(tail->next != NULL) //tail refers to a tail node
    tail = tail->next;

  QUE *n = (QUE *) malloc(sizeof(QUE));
  if (n == NULL)
    return NULL;

  n->next = tail->next;
  tail->next = n;
  n->state = s;  // equivalent to 'tail->next->state = s;'

  return n;
}

/* Dequeue from a head */
struct State dequeue(QUE **p){
  struct State s;
  s.pid = -1;
  if (*p != NULL){
    //printf("DEQ(pid=%d)\n", (*p)->state.pid);
      s = (*p)->state;
      QUE *n = *p;
      *p = n->next;
      free(n);
      return s;
  }else{
    //printf("cannot remove, because queue is empty\n");
    return s;
  }
}

/* Show a queue */
void show(char *str, QUE *n)
{
  printf("%s", str);
  if (n == NULL){
    printf("queue is empty\n");
    return ;
  }
  while (n != NULL){
    printf("[%2d]", n->state.pid);
    n = n->next;
  }
  printf("\n");
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
  struct State init_state;
  
  /* ProcessManager's 6 Data Structures*/
  int time;
  struct Cpu cpu;
  struct PcbTable ptable[MAX_TABLE];
  QUE *ready_states;
  QUE *blocked_states;
  QUE *running_states;
  /**************************************/

  /* Initializing */
  readProgram(fname, prog);
  time = 0;
  cpu.pc = 0;
  cpu.value = 0;
  //ptable[0].pid = pid_cnt;
  
  ready_states = NULL;
  blocked_states = NULL;
  running_states = NULL; // ???
  init_state.pid = pid_cnt;
  enqueue(&running_states, init_state);
  /****************/

  pid_cnt++;

  int n;
  struct State temp_state;
  int temp_value;
  
  while (fgets(buffer, BUFSIZ, fp) != NULL) {
    printf("Instruction=%s",buffer);
    if(!strcmp(buffer, "Q\n")){
      printf("End of one unit of time.\n");
      cmd = split(&n, prog[cpu.pc]);
      printf("cmd[0]=%s\n",cmd[0]);
      
      if(!strcmp(cmd[0],"S")){
	printf("Set the value of the integer variable to %d.\n", atoi(cmd[1]));
	temp_value = cpu.value;
	cpu.value = atoi(cmd[1]);
	printf("cpu.value: %d -> %d\n", temp_value, cpu.value);
	
      }else if(!strcmp(cmd[0],"A")){
	printf("Add %d to the value of the integer variable.\n", atoi(cmd[1]));
	temp_value = cpu.value;
	cpu.value += atoi(cmd[1]);
	printf("cpu.value: %d -> %d\n", temp_value, cpu.value);
	
      }else if(!strcmp(cmd[0],"D")){
	printf("Substract %d from the value of the integer variable.\n", atoi(cmd[1]));
	temp_value = cpu.value;
	cpu.value -= atoi(cmd[1]);
	printf("cpu.value: %d -> %d\n", temp_value, cpu.value);
	
      }else if(!strcmp(cmd[0],"B\n")){
	printf("Block this simulated process.\n");
	temp_state = dequeue(&running_states);
	printf("pid=%d is blocked.\n", temp_state.pid);
	enqueue(&blocked_states, temp_state);
	// TODO: scheduling required.
	
      }else if(!strcmp(cmd[0],"E\n")){
	printf("Terminate this simulated process.\n");
	temp_state = dequeue(&running_states);
	printf("pid=%d is Terminated.\n", temp_state.pid);
	// TODO: scheduling required.
	
      }else if(!strcmp(cmd[0],"F")){
	printf("Create a new simulated process at %d times.\n", atoi(cmd[1]));
	arg = atoi(cmd[1]);
	for(x=0; x<arg; x++){
	  // TODO: create new processes.
	  printf("Created a process with pid=%d.\n", pid_cnt);
	  pid_cnt++;
	}
	
      }else if(!strcmp(cmd[0],"R")){
	printf("Replace the program of the simulated process with the program in the file '%s'.\n", cmd[1]);
	cpu.pc = 0;
	for(x=0;x<MAX_LINE;x++){
	  for(y=0;y<MAX_STR;y++){
	    //prog[x][y] = '\0'; // Not tested
	  }
	}
	//readProgram(cmd[1], prog); // Not tested
	
      }else{
	printf("Unknown Instruction.");	
      }
      
      free(cmd);
      time++;
      cpu.pc++;
      
    }else if(!strcmp(buffer, "U\n")){
      printf("Unblock the first simulated process in blocked queue.\n");
      temp_state = dequeue(&blocked_states);
      if(temp_state.pid == -1){
	printf("There are no states in blocked queue.\n");
      }else{
	printf("pid=%d moves from blocked queue to ready queue.\n", temp_state.pid);
	enqueue(&ready_states, temp_state);
      }
      
    }else if(!strcmp(buffer, "P\n")){
      printf("Print the current state of the system.\n");
      show("Running States: ", running_states);
      show("Ready States: ", ready_states);
      show("Blocked States: ", blocked_states);
      
    }else if(!strcmp(buffer, "T\n")){
      printf("Print the average turnaround time, and terminate the system.\n");
      return;
      
    }else{
      printf("Unknown command.\n");
    }
    //fputs(buffer, stdout);

    printf("\n");
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

