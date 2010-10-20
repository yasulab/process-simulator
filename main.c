#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_STR 256 
#define MAX_LINE 30
#define MAX_TABLE 30
#define MAX_PROCS 256

#define BLOCK 0
#define READY 1
#define RUNNING 2

#define DEBUG true
#define true 1
#define false 0

/* Priority Hierarchy */
#define HIGH 0
#define MED_HIGH 1
#define MED_LOW 2
#define LOW 3

struct Cpu
{
  int pc;
  int pid;
  int value;
  int t_slice;
  int t_remain;
};

struct Proc
{
  int pid;
  int ppid;
  int pc;    // Initially started from zero.
  int value;
  int priority;
  int state;
  int t_start;
  int t_used;
  char fname[MAX_STR]; // Filename to read a program
  char prog[MAX_LINE][MAX_STR]; // Programs for simulated processes.
};

char buffer[BUFSIZ];
//int time;
//int pid_cnt;

typedef struct Que {
  int pt_index;
  struct Proc proc;
  struct Que *next; /* pointer to next element in list */
} QUE;

/* Insert into a head */
QUE *insert_head(QUE **p, int pt_index)
{
  QUE *n = (QUE *) malloc(sizeof(QUE));
  if (n == NULL) return NULL;

  n->next = *p;
  *p = n;
  //n->proc = pt;
  n->pt_index = pt_index;

  return n;
}

/* Enque into a tail */
QUE *enqueue(QUE **p, int pt_index)
{
  QUE *tail = *p;
  //printf("pid=%d\n", s.pid);
  if(*p == NULL) // For initial setup
    return insert_head(p, pt_index);

  while(tail->next != NULL) //tail refers to a tail node
    tail = tail->next;

  QUE *n = (QUE *) malloc(sizeof(QUE));
  if (n == NULL)
    return NULL;

  n->next = tail->next;
  tail->next = n;
  //n->proc = pt;  // equivalent to 'tail->next->proc = pt;'
  n->pt_index = pt_index;

  return n;
}

/* Dequeue from a head */
int dequeue(QUE **p){
  //struct Proc s;
  //s.pid = -1;
  int pt_index = -1;
  if (*p != NULL){
    //printf("DEQ(pid=%d)\n", (*p)->proc.pid);
      pt_index = (*p)->pt_index;
      QUE *n = *p;
      *p = n->next;
      free(n);
      return pt_index;
  }else{
    //printf("cannot remove, because queue is empty\n");
    return pt_index;
  }
}

struct Proc create_proc(int pid, int ppid, int priority, int pc, int value,
			int t_start, int t_used, char *fname){
  static struct Proc proc;
  proc.pid = pid;
  proc.ppid = ppid;
  proc.priority = priority;
  proc.pc = pc;
  proc.value = value;
  proc.t_start = t_start;
  proc.t_used = t_used;
  strcpy(proc.fname, fname);  
  readProgram(proc.fname, proc.prog);
  return proc;
}

struct Proc dup_proc(struct Proc *pp, int new_pid,
		     int dup_times, int current_time){
  static struct Proc cp;
  cp.pid = new_pid;
  cp.ppid = pp->pid;
  cp.priority = pp->priority;
  cp.pc = pp->pc;  // Execute the instruction immediately after F instruction.
  //cp.pc = cp.pc + 1;
  cp.value = pp->value;
  cp.t_start = current_time;
  cp.t_used = 0;
  strcpy(cp.fname, pp->fname);
  readProgram(cp.fname, cp.prog);
  return cp;
} 

/* Show a queue */
void show(QUE *n, struct Proc pcbTable[]){
  struct Proc proc;
  
  if (n == NULL){
    printf("queue is empty\n");
    return ;
  }
  while (n != NULL){
    proc = pcbTable[n->pt_index];
    printf("pid, ppid, priority, value, start time, CPU time used so far\n");
    printf("%3d,  %3d, %8d, %5d, %10d, %3d\n",
	   proc.pid, proc.ppid, proc.priority,
	   proc.value, proc.t_start, proc.t_used);
    n = n->next;
  }
  printf("\n");
  return;
}

void contextSwitch(struct Cpu *cpu, struct Proc *proc){
  struct Cpu temp;
  temp.pc = cpu->pc;
  temp.pid = cpu->pid;
  temp.value = cpu->value;
  temp.t_slice = cpu->t_slice;
  temp.t_remain = cpu->t_remain;

  cpu->pc =  proc->pc;
  cpu->pid = proc->pid;
  cpu->value = proc->value;
  cpu->t_slice = proc->t_start; // ???
  cpu->t_remain = proc->t_used; // ???

  proc->pc = temp.pc;
  proc->pid = temp.pid;
  proc->ppid = 0; // ???
  proc->value = temp.value;
  proc->priority = 0; // ???
  proc->state = READY; // ???
  proc->t_start = temp.t_slice; // ???
  proc->t_used = temp.t_remain; // ???
}

void cpu2proc(struct Cpu *cpu, struct Proc *proc){
  proc->pc = cpu->pc;
  proc->pid = cpu->pid;
  proc->value = cpu->value;
  // proc->t_start =
  //proc->t_used =
  return;
}

int readProgram(char *fname, char prog[][MAX_STR]){
  FILE *fp;
  char buff[MAX_STR], *pp;
  int x, y, i, j;
  
  /* Initialize program arrays */
  for(x=0;x<MAX_LINE;x++){
    for(y=0;y<MAX_STR;y++){
      prog[x][y] = '\0'; // Not tested
    }
  }

  fp = fopen(fname, "r");
  if(fp == NULL){
    printf("Can't open the file: '%s'\n", fname);
    exit(1);
  }

  i=0;
  if(DEBUG) printf("Read '%s' program:\n", fname);
  while(1){
    pp = fgets(buff, MAX_STR, fp);
    
    // delete '\n' character if exists.
    j=0;
    while(buff[j] != '\0'){
      if(buff[j] == '\n') buff[j] = '\0';
      j++;
    }
  
    strcpy(prog[i], buff);
    if(pp == NULL){
      break;
    }
    if(DEBUG) printf("%3d: '%s'\n", i, buff);
    i++;
  }

  fclose(fp);
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

void reporterProcess(struct Proc pcbTable[], int time,
		     QUE *s_run, QUE *s_ready, QUE *s_block){
  printf("*********************************************\n");
  printf("The current system state is as follows:\n");
  printf("*********************************************\n");
  printf("CURRENT TIME: %d\n", time);
  printf("\n");
  printf("RUNNING PROCESS:\n");
  show(s_run, pcbTable);
  //TODO: Formatting the data as following:
  //pid, ppid, priority, value, start time, CPU time used so far
  printf("\n");
  printf("BLOCKED PROCESSES:\n");
  printf("Queue of blocked processes:\n");
  show(s_block, pcbTable);
  //TODO: Formatting the data as following:
  //pid, ppid, priority, value, start time, CPU time used so far
  printf("\n");
  printf("PROCESSES READY TO EXECUTE:\n");
  printf("Queue of processes with priority 0:\n");
  show(s_ready, pcbTable);
  //TODO: Formatting the data as following:
  //pid, ppid, value, start time, CPU time used so far
  
  //show("Queue of processes with priority 1: ", s_ready);
  // ...
  //show("Queue of processes with priority 3: ", s_ready);
  printf("Terminated Reporter Process.\n");
  printf("\n");
  exit(3);
}

void processManagerProcess(int rfd)
{
  FILE *fp = fdopen(rfd, "r");
  //char prog[MAX_LINE][MAX_STR];
  char **cmd;
  int i;
  int pid_count;
  int arg;
  int x,y; // iterators
  
  
  /* ProcessManager's 6 Data Structures*/
  int current_time;
  struct Cpu cpu;
  //struct Proc proc; // Now fixing to the data structure below.
  struct Proc pcbTable[MAX_PROCS];
  int pt_index, pt_count;
  QUE *ready_states;
  QUE *blocked_states;
  QUE *running_states;
  /**************************************/

  /* Initializing */
  pid_count = 0;
  current_time = 0;
  cpu.pc = 0;
  cpu.value = 0;

  pt_index = 0;
  pcbTable[pt_count++] = create_proc(pid_count++, -1, 0, 0, 0,
				     current_time, 0, "init_test");
  
  ready_states = NULL;
  blocked_states = NULL;
  running_states = NULL; // TODO: Re-thinking if needed
  enqueue(&running_states, pt_index);
  /****************/
  if(DEBUG) show(running_states, pcbTable);

  int n;
  struct Proc temp_proc;
  int temp_value;
  int temp_pid;
  char temp_fname[MAX_STR];
  int temp_index;
  
  while (fgets(buffer, BUFSIZ, fp) != NULL) {
    
    printf("Instruction=%s",buffer);
    if(!strcmp(buffer, "Q\n")){
      printf("End of one unit of time.\n");
      if(DEBUG) printf("Next line: %s\n", pcbTable[cpu.pid].prog[cpu.pc]);
      if(DEBUG) printf("pt_index = %d\n", pt_index);
      cmd = split(&n, pcbTable[pt_index].prog[cpu.pc]);
      current_time++;
      cpu.pc++;
      printf("cmd[0]=%s\n",cmd[0]);
      
      if(!strcmp(cmd[0], "S")){
	printf("Set the value of the integer variable to %d.\n", atoi(cmd[1]));
	temp_value = cpu.value;
	cpu.value = atoi(cmd[1]);
	printf("cpu.value: %d -> %d\n", temp_value, cpu.value);
	
      }else if(!strcmp(cmd[0], "A")){
	printf("Add %d to the value of the integer variable.\n", atoi(cmd[1]));
	temp_value = cpu.value;
	cpu.value += atoi(cmd[1]);
	printf("cpu.value: %d -> %d\n", temp_value, cpu.value);
	
      }else if(!strcmp(cmd[0], "D")){
	printf("Substract %d from the value of the integer variable.\n", atoi(cmd[1]));
	temp_value = cpu.value;
	cpu.value -= atoi(cmd[1]);
	printf("cpu.value: %d -> %d\n", temp_value, cpu.value);
	
      }else if(!strcmp(cmd[0], "B")){
	printf("Block this simulated process.\n");
	// Store CPU data to proc
	dequeue(&running_states); 
	cpu2proc(&cpu, &pcbTable[pt_index]);
	printf("Running Process(pid=%d) was blocked.\n", pcbTable[pt_index].pid);
	enqueue(&blocked_states, pt_index);
	// TODO: scheduling required.
	
      }else if(!strcmp(cmd[0], "E")){
	printf("Terminate this simulated process.\n");
	dequeue(&running_states);
	printf("pid=%d is Terminated.\n", pcbTable[pt_index].pid);
	// TODO: scheduling required.
	
      }else if(!strcmp(cmd[0], "F")){
	printf("Create %d new simulated process(es).\n", atoi(cmd[1]));
	arg = atoi(cmd[1]);

	cpu.pc += arg; // Execute N instructions after the next instruction. 
	cpu2proc(&cpu, &pcbTable[pt_index]);
	/* Duplicate a proc and enqueue it into Ready states list. */
	for(x=0; x<arg; x++){
  	  // create new processes.
	  pcbTable[pt_count++] = dup_proc(&pcbTable[pt_index], pid_count++, arg, current_time);
	  enqueue(&ready_states, pt_count-1);
	  printf("Created a process with pid=%d.\n", pid_count-1);
	}
	// Not necessary to schdule processes.
	
      }else if(!strcmp(cmd[0],"R")){
	printf("Replace the program of the simulated process with the program in the file '%s'.\n", cmd[1]);
	strcpy(temp_fname, cmd[1]);
	cpu.pc = 0;
	cpu.value = 0;
	for(x=0;x<MAX_LINE;x++){
	  for(y=0;y<MAX_STR;y++){
	    pcbTable[pt_index].prog[x][y] = '\0'; // Not tested
	  }
	}
	readProgram(temp_fname, pcbTable[pt_index].prog); // Not tested
	printf("Replaced the current program with the program in '%s' file.\n", temp_fname);
	
      }else{
	printf("Unknown Instruction.");	
      }      
      free(cmd);
      
    }else if(!strcmp(buffer, "U\n")){
      printf("Unblock the first simulated process in blocked queue.\n");
      temp_index = dequeue(&blocked_states);
      if(temp_index == -1){
	printf("There are no states in blocked queue.\n");
      }else{
	printf("pid=%d moves from blocked queue to ready queue.\n", temp_index);
	enqueue(&ready_states, temp_index);
      }
      
    }else if(!strcmp(buffer, "P\n")){
      printf("Print the current state of the system.\n");
      if ((temp_pid = fork()) == -1) {
	perror("fork");
      } else if (temp_pid == 0) {
	reporterProcess(pcbTable, current_time,
			running_states, ready_states, blocked_states);
      } else {
	// Do nohing.
      }      
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

