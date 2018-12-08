#include "scheduling_simulator.h"
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <ucontext.h>
void task7(){
	//while(1);
	printf("\n***test***\n");
}
int wait_flag = 1;	//everyone is waiting
void simulate()
{
	wait_flag = 1;
	while(1){
		if(runningQ != NULL){	//return from task itself
			runningQ->state = TASK_TERMINATED;
			enqueue(terminateQ,runningQ);
			runningQ = NULL;
		}
		if(runningQ == NULL){
			if(HreadyQ->front != NULL)
				runningQ = dequeue(HreadyQ);
			else if(LreadyQ->front != NULL)
				runningQ = dequeue(LreadyQ);
			else if(waitingQ->front != NULL){	//when no one in readyQ,but some r still waiting
				while(wait_flag);	//wait til one's waiting time is over,exit while
				wait_flag = 1;
				if(HreadyQ->front != NULL)
					runningQ = dequeue(HreadyQ);
				else if(LreadyQ->front != NULL)
					runningQ = dequeue(LreadyQ);
			}	
			else{
				//printf("hi");
				return;
			}
				
		}
		runningQ->state = TASK_RUNNING;	
		swapcontext(&sim_uc,&(runningQ->uc));
	}
}
int create_task(Queue *Q,char task_name[10],char time,char priority)
{
	//create task_tcb
	TASK_TCB *t = (TASK_TCB*)malloc(sizeof(TASK_TCB));
	strcpy(t->name,task_name);
	t->queueing_time = 0;
	t->pid = ++num;
	t->priority = priority;
	t->time_quantum = time;
	t->state = TASK_READY;
	t->next = NULL;
	enqueue(Q,t);
	getcontext(&(t->uc));
	t->uc.uc_link = &sim_uc;
	t->uc.uc_stack.ss_sp = malloc(50000);
	t->uc.uc_stack.ss_size = 50000;
	if(strcmp(t->name,"task1")==0)
		makecontext(&t->uc,(void(*)(void))task1,0);
	else if(strcmp(t->name,"task2")==0)
		makecontext(&t->uc,(void(*)(void))task2,0);
	else if(strcmp(t->name,"task3")==0)
		makecontext(&t->uc,(void(*)(void))task3,0);
	else if(strcmp(t->name,"task4")==0)
		makecontext(&t->uc,(void(*)(void))task4,0);
	else if(strcmp(t->name,"task5")==0)
		makecontext(&t->uc,(void(*)(void))task5,0);
	else if(strcmp(t->name,"task6")==0)
		makecontext(&t->uc,(void(*)(void))task6,0);
	return t->pid;
}
void enqueue(Queue *Q,TASK_TCB *task)
{
	if(Q->rear == NULL){
		Q->front = Q->rear = task;
		return;
	}
	task->next = NULL;
	Q->rear->next = task;
	Q->rear = task;
}
TASK_TCB *dequeue(Queue *Q)
{
	if(Q->front == NULL)
		return NULL;
	TASK_TCB *temp = Q->front;
	Q->front = Q->front->next;
	if(Q->front == NULL)
		Q->rear = NULL;
	temp->next = NULL;
	return temp;
}
TASK_TCB* remove_task(Queue *Q,int data)
{
	TASK_TCB* temp = Q->front,*prev;
	if(temp != NULL && temp->pid == data){	//if delete node is front
		Q->front = temp->next;
		temp->next = NULL;
		return(temp);
		//return;
	}
	while(temp != NULL && temp->pid != data){	
		prev = temp;
		temp = temp->next;
	}
	if(temp == NULL)	return NULL;	//not exist
	prev->next = temp->next;
	temp->next = NULL;
	if(prev->next == NULL)	//if delete node is rear
		Q->rear = prev;
	return(temp);
}

void cz_signal()
{
	printf("stop\n");
	if(runningQ == NULL)
		swapcontext(&sim_uc,&main_uc);	//stop sim_uculation
	else	
		swapcontext(&(runningQ->uc),&main_uc);
	//signal(SIGTSTP,cz_signal);
}

void set_timer(int n)
{
	struct itimerval tick;
	memset(&tick, 0, sizeof(tick));
	tick.it_value.tv_usec = n;	//10ms
	tick.it_interval.tv_usec = n;
	setitimer(ITIMER_REAL, &tick, NULL);
}
void time_count()	//call in every 10 ms
{
	time++;
	//printf(" %d ",time);
	//++in every  ms
	TASK_TCB *node = HreadyQ->front;
	while(node != NULL){
		node->queueing_time+=10;
		node = node->next;
	}
	node = LreadyQ->front;
	while(node != NULL){
		node->queueing_time+=10;
		node = node->next;
	}
	node = waitingQ->front;
	while(node != NULL){
		node->suspend_time-=10;
		if(node->suspend_time == 0){
			remove_task(waitingQ,node->pid);
			//printf("%p",waitingQ->front);
			if(node->priority == 'H')
				enqueue(HreadyQ,node);
			else if(node->priority == 'L')
				enqueue(LreadyQ,node);
			wait_flag = 0;
		}
		node = node->next;
	}
	
	if(runningQ != NULL){
		if(time==1){
			if(runningQ->time_quantum=='S'){
				TASK_TCB *prev_run = NULL;
				runningQ->state = TASK_READY;
				if(runningQ->priority=='H')
					enqueue(HreadyQ,runningQ);
				else if(runningQ->priority=='L')
					enqueue(LreadyQ,runningQ);
				prev_run = runningQ;
				runningQ = NULL;
				time = 0;
				swapcontext(&(prev_run->uc),&sim_uc);					
			}
		}
		else if (time==2){
			if(runningQ->time_quantum=='L'){
				TASK_TCB *prev_run = NULL;
				runningQ->state = TASK_READY;
				if(runningQ->priority=='H')
					enqueue(HreadyQ,runningQ);
				else if(runningQ->priority=='L')
					enqueue(LreadyQ,runningQ);
				prev_run = runningQ;
				runningQ = NULL;
				time = 0;
				swapcontext(&(prev_run->uc),&sim_uc);					
			}
		}
	}
}

void hw_suspend(int msec_10)
{
	runningQ->state = TASK_WAITING;
	runningQ->suspend_time = msec_10*10;
	enqueue(waitingQ,runningQ);
	runningQ = NULL;
	swapcontext(&(waitingQ->rear->uc),&sim_uc);
	return;
}

void hw_wakeup_pid(int pid)
{
	TASK_TCB *temp = remove_task(waitingQ,pid);
	if(temp->priority == 'H')
		enqueue(HreadyQ,temp);
	else if(temp->priority == 'L')
		enqueue(LreadyQ,temp);
	return;
}

int hw_wakeup_taskname(char *task_name)
{
	int n = 0;
	TASK_TCB *temp,*prev;
	if(waitingQ->front != NULL){
		temp = waitingQ->front;
		while(temp != NULL){
			if(strcmp(temp->name,task_name) == 0){
				prev = temp->next;
				remove_task(waitingQ,temp->pid);
				if(temp->priority == 'H')
					enqueue(HreadyQ,temp);
				else if(temp->priority == 'L')
					enqueue(LreadyQ,temp);
				n++;
			}
			temp = prev;	
		}	
	}
	return n;
}
	

int hw_task_create(char *task_name)
{
	if((strcmp(task_name,"task1")!=0)&&(strcmp(task_name,"task2")!=0)&&(strcmp(task_name,"task3")!=0)
	&&(strcmp(task_name,"task4")!=0)&&(strcmp(task_name,"task5")!=0)&&(strcmp(task_name,"task6")!=0))
		return -1;
	int p = create_task(LreadyQ,task_name,'S','L');
	//printf("hihihi %d\n",p);
	return p;
}

int shell_add(char **argv)
{
	char task_name[20];
	char time_quantum = 'S';
	char priority = 'L';
	strcpy(task_name,argv[1]);
	if(argv[2] != NULL){
		if(strcmp(argv[2],"-t")==0)
			time_quantum = argv[3][0];
		else if(strcmp(argv[2],"-p")==0)
			priority = argv[3][0];
	}
	if(argv[4] != NULL){
		if(strcmp(argv[4],"-t")==0)
			time_quantum = argv[5][0];
		else if(strcmp(argv[4],"-p")==0)
			priority = argv[5][0];
	}
	//create task_tcb
	if(priority == 'H')
		create_task(HreadyQ,task_name,time_quantum,priority);
	else if(priority == 'L')
		create_task(LreadyQ,task_name,time_quantum,priority);
	return 1;
}
int shell_remove(char **argv)
{
	free(remove_task(HreadyQ,atoi(argv[1])));
	free(remove_task(LreadyQ,atoi(argv[1])));
	free(remove_task(waitingQ,atoi(argv[1])));
	free(remove_task(terminateQ,atoi(argv[1])));
	if(runningQ != NULL && (runningQ->pid == atoi(argv[1])))
		runningQ = NULL;
	return 1;
}

int shell_start(char **argv)
{
	//getcontext(&main_uc);
	getcontext(&sim_uc);
	sim_uc.uc_link = &main_uc;
	sim_uc.uc_stack.ss_sp = malloc(50000);
	sim_uc.uc_stack.ss_size = 50000;
	makecontext(&sim_uc,simulate,0);
	
	
	signal(SIGTSTP,cz_signal);	//ctrl+z
	signal(SIGALRM,time_count);
	printf("simulating...\n");
	set_timer(10000);
	
	if(runningQ == NULL)
		swapcontext(&main_uc,&sim_uc);
	else
		swapcontext(&main_uc,&runningQ->uc);

	set_timer(0);	//close timer
	return 1;
}
int shell_ps(char **argv)
{
	TASK_TCB *node = HreadyQ->front;
	if(runningQ != NULL)
		printf("%-2d %s TASK_RUNNING    %-5d %c %c\n",runningQ->pid,runningQ->name,runningQ->queueing_time,runningQ->priority,runningQ->time_quantum);
	while(node != NULL){
		printf("%-2d %s TASK_READY      %-5d %c %c\n",node->pid,node->name,node->queueing_time,node->priority,node->time_quantum);
		node = node->next;
	}
	node = LreadyQ->front;
	while(node != NULL){
		printf("%-2d %s TASK_READY      %-5d %c %c\n",node->pid,node->name,node->queueing_time,node->priority,node->time_quantum);
		node = node->next;
	}
	node = waitingQ->front;
	while(node != NULL){
		printf("%-2d %s TASK_WAITING    %-5d %c %c\n",node->pid,node->name,node->queueing_time,node->priority,node->time_quantum);
		node = node->next;
	}
	node = terminateQ->front;
	while(node != NULL){
		printf("%-2d %s TASK_TERMINATED %-5d %c %c\n",node->pid,node->name,node->queueing_time,node->priority,node->time_quantum);
		node = node->next;
	}
	return 1;
}

/*launch a program and wait for it to terminate*/
int shell_launch(char **argv)
{
	pid_t pid;
	int status;
	pid = fork();
	if (pid == 0) {
		// Child process
		if (execvp(argv[0], argv) == -1) {
			perror("shell");
		}
		exit(EXIT_FAILURE);
	} else if (pid < 0) {
		// Error forking
		perror("shell");
	} else {
		// Parent process
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

/*find if command exist and call shell_launch to execute*/
int shell_execute(char **argv)
{
	int i;
	if (argv[0] == NULL) // An empty command was entered.
		return 1;
	for (i = 0; i < 4; i++) {
		if (strcmp(argv[0], builtin_str[i]) == 0) {
			return (*builtin_func[i])(argv);
		}
	}
	return shell_launch(argv);
}

/*read a line of input from stdin */
#define shell_RL_BUFSIZE 1024
char *shell_read_line(void)
{
	int bufsize = shell_RL_BUFSIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * bufsize);
	int c;
	if (!buffer) {
		fprintf(stderr, "shell: allocation error\n");
		exit(EXIT_FAILURE);
	}
	while (1) {
		// Read a character
		c = getchar();
		if (c == EOF) {
			exit(EXIT_SUCCESS);
		} else if (c == '\n') {
			buffer[position] = '\0';
			return buffer;
		} else {
			buffer[position] = c;
		}
		position++;
		// If we have exceeded the buffer, reallocate.
		if (position >= bufsize) {
			bufsize += shell_RL_BUFSIZE;
			buffer = realloc(buffer, bufsize);
			if (!buffer) {
				fprintf(stderr, "shell: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

/*split a line into tokens */
#define shell_TOK_BUFSIZE 64
#define shell_TOK_DELIM " \t\r\n\a"
char **shell_split_line(char *line)
{
	int bufsize = shell_TOK_BUFSIZE, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token, **tokens_backup;
	if (!tokens) {
		fprintf(stderr, "shell: allocation error\n");
		exit(EXIT_FAILURE);
	}
	token = strtok(line, shell_TOK_DELIM);
	while (token != NULL) {
		tokens[position] = token;
		position++;
		if (position >= bufsize) {
			bufsize += shell_TOK_BUFSIZE;
			tokens_backup = tokens;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens) {
				free(tokens_backup);
				fprintf(stderr, "shell: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, shell_TOK_DELIM);
	}
	tokens[position] = NULL;
	return tokens;
}

void shell_loop(void)
{
	char *line;
	char **argv;
	int status;
	do {
		printf("$ ");
		line = shell_read_line();	//read the command
		argv = shell_split_line(line);	//spilt the command into func and arg
		status = shell_execute(argv);
		free(line);
		free(argv);
	} while (status);
}

int main(int argc, char **argv)
{
	initQ();
	shell_loop();
	return EXIT_SUCCESS;
}