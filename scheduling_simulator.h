#ifndef SCHEDULING_SIMULATOR_H
#define SCHEDULING_SIMULATOR_H

#include <stdio.h>
#include <ucontext.h>

#include "task.h"

ucontext_t main_uc,sim_uc,end_uc;
char stack[1024*128];

typedef enum TASK_STATE{
	TASK_RUNNING,
	TASK_READY,
	TASK_WAITING,
	TASK_TERMINATED
}TASK_STATE;

typedef struct TASK_TCB{
	ucontext_t uc;
	TASK_STATE state;
	int pid;
	char name[10];
	char priority;
	int queueing_time;
	int suspend_time;
	char time_quantum;
	struct TASK_TCB *next;
}TASK_TCB;

typedef struct Queue{
	TASK_TCB *front,*last;
}Queue;

/*simulate mode*/
void hw_suspend(int msec_10);
void hw_wakeup_pid(int pid);
int hw_wakeup_taskname(char *task_name);
int hw_task_create(char *task_name);

/*shell mode*/
int shell_add(char **argv);
int shell_start(char **argv);
int shell_ps(char **argv);
int shell_remove(char **argv);

int shell_launch(char **argv);
int shell_execute(char **argv);
char *shell_read_line(void);
char **shell_split_line(char *line);
void shell_loop(void);
char *builtin_str[] = {
	"add",
	"start",
	"ps",
	"remove"
};
int (*builtin_func[]) (char **) = {
  	&shell_add,
	&shell_start,
	&shell_ps,
	&shell_remove
};

/*queue operation*/
int num = 0;	//for pid
Queue *HreadyQ = NULL;
Queue *LreadyQ = NULL;
Queue *waitingQ = NULL;
Queue *terminateQ = NULL;
TASK_TCB *runningQ = NULL;
TASK_TCB *prev_runningQ = NULL;
void enqueue(Queue *Q,TASK_TCB *task);
TASK_TCB *dequeue(Queue *Q);
int create_task(Queue *Q,char task_name[10],char time,char priority);
TASK_TCB* remove_task(Queue *Q,int data);
void initQ()
{
	HreadyQ = (Queue*)malloc(sizeof(Queue));
	HreadyQ->front = HreadyQ->last = NULL;
	LreadyQ = (Queue*)malloc(sizeof(Queue));
	LreadyQ->front = LreadyQ->last = NULL;
	waitingQ = (Queue*)malloc(sizeof(Queue));
	waitingQ->front = waitingQ->last = NULL;
	terminateQ = (Queue*)malloc(sizeof(Queue));
	terminateQ->front = terminateQ->last = NULL;
}

/*signal*/
int time = 0;
void cz_signal();	//crtl+z
void set_timer();	//timer init
void time_count();

#endif
