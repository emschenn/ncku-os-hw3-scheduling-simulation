#ifndef SCHEDULING_SIMULATOR_H
#define SCHEDULING_SIMULATOR_H

#include <stdio.h>
#include <ucontext.h>

#include "task.h"


typedef enum TASK_STATE{
	TASK_RUNNING,
	TASK_READY,
	TASK_WAITING,
	TASK_TERMINATED
}TASK_STATE;

typedef struct TASK_TCB{
	int pid;
	char name[10];
	TASK_STATE state;
	int queueing_time;
	int time_quantum;
	struct TASK_TCB *next;
}TASK_TCB;

typedef struct Queue{
	//int size;
	TASK_TCB *front,*last;
}Queue;



//simulate mode
void hw_suspend(int msec_10);
void hw_wakeup_pid(int pid);
int hw_wakeup_taskname(char *task_name);
int hw_task_create(char *task_name);

//shell mode
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

#endif
