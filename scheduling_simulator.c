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
Queue *HQ = NULL;
Queue *LQ = NULL;
int num = 0;
void initQ()
{
	HQ = (Queue*)malloc(sizeof(Queue));
	HQ->front = HQ->last = NULL;
	LQ = (Queue*)malloc(sizeof(Queue));
	LQ->front = LQ->last = NULL;
}
void enqueue(Queue *Q,TASK_TCB *task)
{
	if(Q->last == NULL){
		Q->front = Q->last = task;
		return;
	}
	Q->last->next = task;
	Q->last = task;
}
TASK_TCB *dequeue(Queue *Q)
{
	if(Q->front == NULL)
		return NULL;
	TASK_TCB *temp = Q->front;
	Q->front = Q->front->next;
	if(Q->front == NULL)
		Q->last = NULL;
	return temp;
}
void delete(Queue *Q,int data)
{
	TASK_TCB* temp = Q->front,*prev;
	if(temp != NULL && temp->pid == data){
		Q->front = temp->next;
		free(temp);
		return;
	}
	while(temp != NULL && temp->pid != data){
		prev = temp;
		temp = temp->next;
	}
	if(temp == NULL)	return;	//not exist
	prev->next = temp->next;
	free(temp);
}
void handler(int signal)
{
	printf("%d stop",signal);
}
void set_timer()
{
	struct itimerval tick;
	memset(&tick, 0, sizeof(tick));
	tick.it_value.tv_usec = 10;
	tick.it_interval.tv_usec = 10;
	setitimer(ITIMER_REAL, &tick, NULL);
}
void hw_suspend(int msec_10)
{
	return;
}

void hw_wakeup_pid(int pid)
{
	return;
}

int hw_wakeup_taskname(char *task_name)
{
    return 0;
}

int hw_task_create(char *task_name)
{
	printf("hi\n");
    return 0; // the pid of created task name
}

int shell_add(char **argv)
{
	char task_name[20];
	char time_quantum = 'S';
	char priority = 'H';
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
	printf("%s ,%c, %c",task_name,time_quantum,priority);
	//create task_tcb
	TASK_TCB *t = (TASK_TCB*)malloc(sizeof(TASK_TCB));
	strcpy(t->name,task_name);
	t->queueing_time = 0;
	t->pid = ++num;
	if(time_quantum == 'L')	t->time_quantum = 20;
	else if(time_quantum == 'S')	t->time_quantum = 10;
	t->state = TASK_READY;
	//put into queue
	if(priority == 'H')	enqueue(HQ,t);
	else if(priority == 'L')	enqueue(LQ,t);
	return 1;
}
int shell_remove(char **argv)
{
	delete(HQ,atoi(argv[1]));
	delete(LQ,atoi(argv[1]));
	printf("remove %d",atoi(argv[1]));
	return 1;
}
int shell_start(char **argv)
{
	signal(SIGALRM,&hw_task_create);
	set_timer();
	while(1){}	
	printf("start");
	return 1;
}
int shell_ps(char **argv)
{
	TASK_TCB *node = HQ->front;
	while(node != NULL){
		printf("H %d %s \n",node->pid,node->name);
		node = node->next;
	}
	node = LQ->front;
	while(node != NULL){
		printf("L %d %s \n",node->pid,node->name);
		node = node->next;
	}
	return 1;
}
/**
  @brief Launch a program and wait for it to terminate.
  @param argv Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
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
	signal(SIGTSTP,&handler);	//ctrl+z
	do {
		printf("$ ");
		line = shell_read_line();	//read the command
		argv = shell_split_line(line);	//spilt the command into func and arg
		status = shell_execute(argv);
		free(line);
		free(argv);
		printf("\n");
	} while (status);
}

int main(int argc, char **argv)
{
	initQ();
	shell_loop();
	return EXIT_SUCCESS;
}