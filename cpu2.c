#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#define MAX_PROCESSES 20
typedef struct process{
	int pid;
	int arrivalTime;
	int burstTime;
	int startTime;
	int endTime;
	int waitingTime;
	int turnArroundTime;	
	int priority;
	int quantumRemaining;
	int currentQueue;
} Process;

typedef struct process_node{
	struct process *data;
	struct process_node *next;
} Process_node;

typedef struct process_queue{
	int size;
	struct process_node *front;
	struct process_node *back;
} Process_queue;

Process processes[MAX_PROCESSES+1];
// Temporary "Pre-Ready" queue
Process *tmpQueue[MAX_PROCESSES+1];
int numberOfProcesses;
int theClock;
int totalWaitingTime;
int totalTurnArroundTime;
float avgWaitingTime;
float avgTurnArroundTime;


// Ready process queue
Process_queue readyQueue;

/* Queue management functions */
Process_node *createProcessNode(Process *);
void initializeProcessQueue(Process_queue *);
void enqueueProcess(Process_queue *, Process *);
void dequeueProcess(Process_queue *);
/**
 * Resets all global variables to 0 
 */
void resetVariables(void){
	numberOfProcesses = 0;	
	totalWaitingTime = 0;	
	theClock = 0;
	totalWaitingTime = 0;
	totalTurnArroundTime=0;
	avgWaitingTime=0;
	avgTurnArroundTime=0;
}

void accept(int ch)
{
int i;
printf("\n\tHow many processes: ");
scanf("%d",&numberOfProcesses);
printf("\tEnter the values\n");
if(ch==1 || ch==2)
{
printf("\n\tArrival Time and Burst Time\n");
for(i=0;i<numberOfProcesses;i++)
{
printf("\tEnter for Process %d :",i);
scanf("%d%d",&processes[i].arrivalTime,&processes[i].burstTime);
processes[i].pid=i;
}
}
if(ch==3)
{
printf("\n\tArrival Time , Burst Time and Priority\n");
for(i=0;i<numberOfProcesses;i++)
{
printf("\tEnter for Process %d :",i);
scanf("%d%d%d",&processes[i].arrivalTime,&processes[i].burstTime,&processes[i].priority);
processes[i].pid=i;
}
}
if(ch==4)
{
printf("\n\tArrival Time and Burst Time\n");
for(i=0;i<numberOfProcesses;i++)
{
printf("\tEnter for Process %d :",i);
scanf("%d%d",&processes[i].arrivalTime,&processes[i].burstTime);
processes[i].pid=i;
}
}
}

/**
 * Creates a single process node with pointer to data and next
 */
Process_node *createProcessNode(Process *p){
	Process_node *node = (Process_node*)malloc(sizeof(Process_node));
	if (node == NULL){
		printf("out of memory");
        exit(-1);
	}
	node->data = p;
	node->next = NULL;
	return node;
}

/**
 * Initializes a process queue. Makes an empty queue
 */
void initializeProcessQueue(Process_queue *q){
	q = (Process_queue*)malloc(sizeof(Process_queue));
	q->front = q->back = NULL;
	q->size = 0;
}

/**
 * Compare arrival time of two processes
 */
int compareArrivalTime(const void *a, const void *b){
	Process *first = (Process *) a;
	Process *second = (Process *) b;
	return first->arrivalTime - second->arrivalTime;
}

/**
 * Compare process ID of two processes
 */
/*int compareProcessIds(const void *a, const void *b){
	Process *first = (Process *) a;
	Process *second = (Process *) b;
	if (first->pid == second->pid){
		error_duplicate_pid(first->pid);
	}
	return first->pid - second->pid;
}*/

/**
 * Equeues a process
 */
void enqueueProcess(Process_queue *q, Process *p){
	Process_node *node = createProcessNode(p);
	if (q->front == NULL){		
		q->front = q->back = node;
	}
	else{		
		q->back->next = node;
		q->back = node;
	}
	q->size++;
}
/**
 * Dequeues a process
 */
void dequeueProcess(Process_queue *q) {
    Process_node *deleted = q->front;    
    if (q->size == 1) {
        q->front = NULL;
        q->back = NULL;
    } else {       
        q->front = q->front->next;
    }
    free(deleted);
    q->size--;  
}
/**
 * Add any new incoming processes to a temporary queue to be sorted and later added
 * to the ready queue. These incoming processes are put in a "pre-ready queue"
 */
void addNewIncomingProcess(void){
    int i=0;
	while(i < numberOfProcesses){	   
		tmpQueue[i] = &processes[i];
		i++;
	}
}
/**
 * Grabs the next scheduled process in the queue (first process currently at
 * the front of the ready queue). Increments the waiting time in order to update
 * the ready state. Returns the next process to be run
 */
Process *nextScheduledProcess(void){
	if (readyQueue.size == 0){
		return NULL;
	}
	//printf("readyQueue.front->data->pid %d",readyQueue.front->data->pid);
	Process *grabNext = readyQueue.front->data;
	dequeueProcess(&readyQueue);
	return grabNext;
}

void *fcfs(void *p)
{
	int i;
	float avgtat=0,avgwt=0;

	resetVariables();
	initializeProcessQueue(&readyQueue);	
	accept(1);
	qsort(processes, numberOfProcesses, sizeof(Process*), compareArrivalTime);

    addNewIncomingProcess();
	// enqueue processes to readyqueue
	for(i=0;i<numberOfProcesses;i++)
	{
		enqueueProcess(&readyQueue, tmpQueue[i]);
	}
	theClock=0;
	//Run processes
	Process *currProcess;
	//currProcess=malloc(sizeof(Process));
	
		
	while(readyQueue.size > 0){	   
		
		currProcess = nextScheduledProcess();//get the next process at the front of ready queue
		printf("\n\t=============================================================================================");
		// Set the clock to new value - only if current process has arrival time greater than the clock		
		theClock=theClock>currProcess->arrivalTime?theClock:currProcess->arrivalTime;	
		currProcess->startTime=theClock; // consider the current process execution just started


		printf("\n\t Starting the process %d ",currProcess->pid);
		printf("\n\t time on the clock is now : %d  ",theClock);
		printf("\n\t Process %d arrived at %d on the clock",currProcess->pid,currProcess->arrivalTime);

		currProcess->waitingTime=currProcess->startTime-currProcess->arrivalTime;
		printf("\n\t Waiting time for process %d = %d",currProcess->pid,currProcess->waitingTime);

		theClock=theClock+currProcess->burstTime; // Set the clock to new value, by adding the time taken to run current process (considering current process just finshed)
		currProcess->endTime=theClock;
		currProcess->turnArroundTime=currProcess->endTime-currProcess->arrivalTime;

		printf("\n\t End of process %d ",currProcess->pid);
		printf("\n\t time on the clock is now : %d  ",theClock);
		printf("\n\t Turn Around Time for process %d = %d ",currProcess->pid,currProcess->turnArroundTime);

		totalTurnArroundTime=totalTurnArroundTime+currProcess->turnArroundTime;
		totalWaitingTime=totalWaitingTime+currProcess->waitingTime;
		printf("\n\t================================================================================================");
	}

	avgTurnArroundTime=totalTurnArroundTime/numberOfProcesses;
	avgWaitingTime=totalWaitingTime/numberOfProcesses;    

	printf("\n\t Average Turn Around Time:%f ",avgTurnArroundTime);
	printf("\n\t Average waiting time:%f ",avgWaitingTime);
	
}

void main()
{
pthread_t thread;
int ch,tq;
int rc;

	printf("\n=============================================================================================================================");
	printf("\n\t —- Scheduling Algorithms —-");
	printf("\n\t1. FCFS\n   \t2. Round Robin \n \t3. Priority Based\n  \t4. Exit");
	printf("\n\t Enter your choice: ");
	scanf("%d",&ch);
	switch(ch)
{
	case 1: 
			//fcfs();
			pthread_create(&thread,NULL,fcfs,(void *) NULL);	// not passing anything
			if (rc){ printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
            }
			
			break;
	case 2: break;
	case 3: break;
	case 4: break;


}
printf("\n\n\tPress any key to continue………");
pthread_exit(NULL);


//getch();
}

