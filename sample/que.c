#include <stdio.h> /* for printf */
#include <stdlib.h> /* for malloc */

struct State{
  int pid;
};



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
  printf("pid=%d\n", s.pid);
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
      printf("DEQ(pid=%d)\n", (*p)->state.pid);
      s = (*p)->state;
      QUE *n = *p;
      *p = n->next;
      free(n);
      return s;
  }else{
    printf("cannot remove, because queue is empty\n");
    return s;
  }
}

/* Show a queue */
void show(QUE *n)
{
  printf("QUEUE : ");
  if (n == NULL)
    {
      printf("queue is empty\n");
      return ;
    }
  while (n != NULL)
    {
      printf("[%2d]", n->state.pid);
      n = n->next;
    }
  printf("\n\n");
}

int main(void)
{
  QUE *n = NULL;
  struct State test1;
  struct State test2;
  struct State test3;
  struct State test4;
  struct State test5;
  test1.pid = 1;
  test2.pid = 2;
  test3.pid = 3;
  test4.pid = 4;
  test5.pid = 5;


  show(n);
  enqueue(&n, test1);
  show(n);
  enqueue(&n, test2);
  show(n);
  enqueue(&n, test3);
  show(n);
  enqueue(&n, test4);
  show(n);
  enqueue(&n, test5);
  show(n);
  
  struct State s1 = dequeue(&n);
  show(n);
  struct State s2 = dequeue(&n);
  show(n);
  struct State s3 = dequeue(&n);
  show(n);
  struct State s4 = dequeue(&n);
  show(n);
  struct State s5 = dequeue(&n);
  show(n);

  printf("s1.pid=%d\n", s1.pid);
  printf("s2.pid=%d\n", s2.pid);
  printf("s3.pid=%d\n", s3.pid);
  printf("s4.pid=%d\n", s4.pid);
  printf("s5.pid=%d\n", s5.pid);

  return 0;
}
