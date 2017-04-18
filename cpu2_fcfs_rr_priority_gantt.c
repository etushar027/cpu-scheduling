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
	int remainingBurst;
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

typedef struct ganttChartStep{
	int pid;
	int clock;	
} GanttChartStep;
GanttChartStep ganttChartStepArray[200];

Process processes[MAX_PROCESSES+1];
// Temporary "Pre-Ready" queue
Process *tmpQueue[MAX_PROCESSES+1];
int numberOfProcesses;
int theClock;
int timeQuantum;
int totalWaitingTime;
int totalTurnArroundTime;
float avgWaitingTime;
float avgTurnArroundTime;

unsigned int time_width = 4;


// Ready process queue
Process_queue readyQueue;

/* Queue management functions */
Process_node *createProcessNode(Process *);
void initializeProcessQueue(Process_queue *);
void enqueueProcess(Process_queue *, Process *);
void dequeueProcess(Process_queue *);
void dispalyGanttChart();
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
processes[i].remainingBurst=processes[i].burstTime;
processes[i].endTime=processes[i].arrivalTime;
}
}
else if(ch==3)
{
printf("\n\tArrival Time , Burst Time and Priority\n");
for(int i=0;i<numberOfProcesses;i++)
{
printf("\tEnter for Process %d :",i);
processes[i].pid=i;
scanf("%d%d%d",&processes[i].arrivalTime,&processes[i].burstTime,&processes[i].priority);
processes[i].remainingBurst=processes[i].burstTime;
processes[i].endTime=processes[i].arrivalTime;
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
 * Compare arrival time of two processes
 */
int comparePriorityAndArrivalTime(const void *a, const void *b){
	Process *first = (Process *) a;
	Process *second = (Process *) b;
	int priorityDiff=first->priority - second->priority;
	if(priorityDiff==0)
	{
	    return first->arrivalTime - second->arrivalTime;
	}
	else{
	    return -priorityDiff;
	}
	
	
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
	Process *grabNext = readyQueue.front->data;
	/*//TODO
	printf("\n inside nextScheduledProcess --- pid %d , burst time %d",grabNext->pid,grabNext->burstTime);*/
	dequeueProcess(&readyQueue);
	/*printf("\n inside nextScheduledProcess --- pid %d , burst time %d",grabNext->pid,grabNext->burstTime);*/
	return grabNext;
}


void *priority_aging(void *p)
{
	int i,step,noOfPendingProcess;
	float avgtat=0,avgwt=0;

	resetVariables();
	initializeProcessQueue(&readyQueue);
	
    
	accept(3);
	noOfPendingProcess=numberOfProcesses;
	qsort(processes, numberOfProcesses, sizeof(Process), comparePriorityAndArrivalTime);// sort based on priority and arrival time
    
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
	
	printf("\n\t=============================================================================================\n");
	while(readyQueue.size > 0)
	{
		Process *currProcess;
		currProcess = nextScheduledProcess();//get the next process at the front of ready queue
		// Set the clock to new value - only if current process has arrival time greater than the clock	
		if(theClock==0)
		  currProcess->arrivalTime=theClock;
		//theClock=theClock>currProcess->arrivalTime?theClock:currProcess->arrivalTime;	
		currProcess->startTime=theClock; // consider the current process execution just started
		printf("\n\t Starting the process %d ",currProcess->pid);
		printf("\n\t time on the clock is now : %d  ",theClock);
		printf("\n\t Process %d arrived at %d on the clock",currProcess->pid,currProcess->arrivalTime);
		currProcess->waitingTime=currProcess->startTime-currProcess->arrivalTime;
		printf("\n\t Waiting time for process %d = %d",currProcess->pid,currProcess->waitingTime);

        //printf("\n\t burstTime time for process %d , burstTime = %d",currProcess->pid,currProcess->burstTime);
        
		theClock=theClock+currProcess->burstTime; // Set the clock to new value, by adding the time taken to run current process (considering current process just finshed)
		currProcess->endTime=theClock;
		currProcess->turnArroundTime=currProcess->endTime-currProcess->arrivalTime;
		printf("\n\t End of process %d ",currProcess->pid);
		printf("\n\t time on the clock is now : %d  ",theClock);
		printf("\n\t Turn Around Time for process %d = %d ",currProcess->pid,currProcess->turnArroundTime);
		
		totalTurnArroundTime=totalTurnArroundTime+currProcess->turnArroundTime;
		totalWaitingTime=totalWaitingTime+currProcess->waitingTime;
		printf("\n\t============================================================================================");
		
		
		//----------------preapring steps for gantt chart-----------
		ganttChartStepArray[step].pid=currProcess->pid;
		ganttChartStepArray[step].clock=theClock;
		step++;	
		
		/*noOfPendingProcess--; // decreasing by 1 as one process executed already
		Process pendingProcArray[noOfPendingProcess];// declaring array to be used for sorting
		Process *pendingProcPointerArray[noOfPendingProcess];// declaring array to be used for sorting
		int j=0;
		while(readyQueue.size > 0)
		{
		    
		    currProcess = nextScheduledProcess();//get the next process at the front of ready queue
		    currProcess->waitingTime=theClock-currProcess->arrivalTime; // waiting time so far
		    currProcess->priority=currProcess->priority+currProcess->waitingTime; // increase priority by the waiting timing so far		    
		    pendingProcArray[j].arrivalTime=currProcess->arrivalTime;	
		    pendingProcArray[j].pid=currProcess->pid;
		    pendingProcArray[j].priority=currProcess->priority;
		    pendingProcArray[j].burstTime=currProcess->burstTime;
		    pendingProcArray[j].waitingTime=currProcess->waitingTime;
		    
		    j++;
		}
		
		qsort(pendingProcArray, noOfPendingProcess, sizeof(Process), comparePriorityAndArrivalTime);// sort based on priority and arrival time

        //TODO
        printf("\n noOfPendingProcess %d",noOfPendingProcess);
        for(int i=0;i<noOfPendingProcess;i++)
        {
            pendingProcPointerArray[i]=&pendingProcArray[i];
            printf("\n i %d , pid %d , priority %d , burstTime %d",i,pendingProcPointerArray[i]->pid , pendingProcPointerArray[i]->priority, pendingProcPointerArray[i]->burstTime);//TODO
        }

        //addNewIncomingProcess();
    	// enqueue processes to readyqueue
    	//initializeProcessQueue(&readyQueue);
    	printf("\n readyQueue.size %d ", readyQueue.size);//TODO
    	for(int i=0;i<noOfPendingProcess;i++)
    	{
    		
    		enqueueProcess(&readyQueue, pendingProcPointerArray[i]);
    		printf("\n readyQueue.size %d ", readyQueue.size);//TODO
    	}
    	*/
		
		
	}

	avgTurnArroundTime=totalTurnArroundTime/numberOfProcesses;
	avgWaitingTime=totalWaitingTime/numberOfProcesses; 
	
	 printf("\n\n");
	dispalyGanttChart();
    printf("\n\n");
	printf("\n\t Average Turn Around Time:%f ",avgTurnArroundTime);
	printf("\n\t Average waiting time:%f ",avgWaitingTime);
	printf("\n\n");
	pthread_exit(NULL);
	
}


void *fcfs(void *p)
{
	int i,step;
	float avgtat=0,avgwt=0;

	resetVariables();
	initializeProcessQueue(&readyQueue);	
	accept(1);
	qsort(processes, numberOfProcesses, sizeof(Process), compareArrivalTime);

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
		printf("\n\t===================================================================");
		// Set the clock to new value - only if current process has arrival time greater than the clock		
		//theClock=theClock>currProcess->arrivalTime?theClock:currProcess->arrivalTime;	
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
		
		
		//----------------preapring steps for gantt chart-----------
		ganttChartStepArray[step].pid=currProcess->pid;
		ganttChartStepArray[step].clock=theClock;
		step++;
	}

	avgTurnArroundTime=totalTurnArroundTime/numberOfProcesses;
	avgWaitingTime=totalWaitingTime/numberOfProcesses;    

	printf("\n\n");
	dispalyGanttChart();
    printf("\n\n");
	printf("\n\t Average Turn Around Time:%f ",avgTurnArroundTime);
	printf("\n\t Average waiting time:%f ",avgWaitingTime);
	printf("\n\n");
	
}

void *roundRobin(void *p)
{
	int i,step;
	float avgtat=0,avgwt=0;

	resetVariables();
	initializeProcessQueue(&readyQueue);	
	printf("\n\tEnter the Time Quantum: ");
	scanf("%d",&timeQuantum);
    
	accept(2);
	qsort(processes, numberOfProcesses, sizeof(Process), compareArrivalTime);

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
	
		printf("\n\t=============================================================================================\n");
	while(readyQueue.size > 0){	   
		
		currProcess = nextScheduledProcess();//get the next process at the front of ready queue
		
		// Set the clock to new value - only if current process has arrival time greater than the clock		
		//theClock=theClock>currProcess->arrivalTime?theClock:currProcess->arrivalTime;	
		currProcess->startTime=theClock; // consider the current process execution just started //remainingBurst


		printf("\n\t Starting the process %d ",currProcess->pid);
		printf("\n\t time on the clock is now : %d  ",theClock);
		
		printf("\n\t currProcess->startTime %d ",currProcess->startTime);
		printf("\n\t currProcess->endTime %d ",currProcess->endTime);
		printf("\n\t Process %d arrived at %d on the clock",currProcess->pid,currProcess->arrivalTime);

        //waiting time is --> waiting time so far the process + current start time - last end time of the process
		currProcess->waitingTime=currProcess->startTime-currProcess->endTime;
		printf("\n\t Waiting time so far for the process %d = %d",currProcess->pid,currProcess->waitingTime);

        
		if(currProcess->remainingBurst > timeQuantum)
		{
		    currProcess->remainingBurst=currProcess->remainingBurst-timeQuantum;
		    //enqueing at the back of the queue to be executed later- queue size increases by 1
		    enqueueProcess(&readyQueue, currProcess);
		    theClock=theClock+timeQuantum; // Set the clock to new value, by adding the time taken to run current process (considering current process just finshed)
		}
		else
		{
		    theClock=theClock+currProcess->remainingBurst; // Set the clock to new value, by adding the time taken to run current process (considering current process just finshed)
		    currProcess->remainingBurst=0;
		}
		
		currProcess->endTime=theClock;
		currProcess->turnArroundTime=currProcess->endTime-currProcess->arrivalTime;

		printf("\n\t End of process %d ",currProcess->pid);
		printf("\n\t time on the clock is now : %d  ",theClock);
		printf("\n\t Turn Around Time for process %d = %d ",currProcess->pid,currProcess->turnArroundTime);

		totalTurnArroundTime=totalTurnArroundTime+currProcess->turnArroundTime;
		totalWaitingTime=totalWaitingTime+currProcess->waitingTime;
		printf("\n\t totalWaitingTime :%d ",totalWaitingTime);
		printf("\n\t================================================================================================");
		
		
		
		
		//----------------preapring steps for gantt chart-----------
		ganttChartStepArray[step].pid=currProcess->pid;
		ganttChartStepArray[step].clock=theClock;
		step++;
		
		
		
		
	}

	avgTurnArroundTime=totalTurnArroundTime/numberOfProcesses;
	avgWaitingTime=totalWaitingTime/numberOfProcesses; 
	
	 printf("\n\n");
	dispalyGanttChart();
    printf("\n\n");
	printf("\n\t Average Turn Around Time:%f ",avgTurnArroundTime);
	printf("\n\t Average waiting time:%f ",avgWaitingTime);
	printf("\n\n");
	pthread_exit(NULL);
	
}



void dispalyGanttChart()
{
    printf("========Here goes the gantt chart========\n");
    
    //1st line 
    // print  '-' equals to the time taken for execution of each step
    printf("-");
    for(int step=0;step<200;step++)//for each step
    {
        if(ganttChartStepArray[step].pid==0 && ganttChartStepArray[step].clock==0)//this marks end of all steps , so break
            break;
        else // continue
        {
            int executionlength;
            if(step==0)
            {
                executionlength=ganttChartStepArray[step].clock;
            }
            else
            {
                executionlength=ganttChartStepArray[step].clock-ganttChartStepArray[step-1].clock;
            }
            
            for(int x=0;x<executionlength*time_width;x++)
            {
		       printf("-");
		    }
    		
        }
    }
    
      
    //2nd line
    //print process ids , use spaces to align the process id at the middle
    printf("\n");
    printf("|");
    for(int step=0;step<200;step++)
    {
        if(ganttChartStepArray[step].pid==0 && ganttChartStepArray[step].clock==0)//this marks end of all steps , so break
            break;
        else//continue
        {
            int executionlength;
            if(step==0)
            {
                executionlength=ganttChartStepArray[step].clock;
            }
            else
            {
                executionlength=ganttChartStepArray[step].clock-ganttChartStepArray[step-1].clock;
            }           
            
    		unsigned z=executionlength*time_width;
    		printf("%*u", z - (z/2), ganttChartStepArray[step].pid);
    		printf("%*c", z/2, '|' );
        }
    }
    
    
    //3rd line 
    // print  '-' equals to the time taken for execution of each step
    printf("\n");
    printf("-");
    for(int step=0;step<200;step++)//for each step
    {
        if(ganttChartStepArray[step].pid==0 && ganttChartStepArray[step].clock==0)//this marks end of all steps , so break
            break;
        else // continue
        {
            int executionlength;
            if(step==0)
            {
                executionlength=ganttChartStepArray[step].clock;
            }
            else
            {
                executionlength=ganttChartStepArray[step].clock-ganttChartStepArray[step-1].clock;
            }
            
            for(int x=0;x<executionlength*time_width;x++)
            {
		       printf("-");
		    }
    		
        }
    }
    
    //4th line
    //print | at end of each step , use spaces to align the | at the right end
    printf("\n");
    printf("|");
    for(int step=0;step<200;step++)
    {
        if(ganttChartStepArray[step].pid==0 && ganttChartStepArray[step].clock==0)//this marks end of all steps , so break
            break;
        else//continue
        {
            int executionlength;
            if(step==0)
            {
                executionlength=ganttChartStepArray[step].clock;
            }
            else
            {
                executionlength=ganttChartStepArray[step].clock-ganttChartStepArray[step-1].clock;
            }           
            
    		unsigned z=executionlength*time_width;    		
    		printf("%*c", z, '|' );
        }
    }
    
    //5th line
    //print clock , use spaces to align the process at the right end
    printf("\n");
    printf("0");
    for(int step=0;step<200;step++)
    {
        if(ganttChartStepArray[step].pid==0 && ganttChartStepArray[step].clock==0)//this marks end of all steps , so break
            break;
        else//continue
        {
            int executionlength;
            if(step==0)
            {
                executionlength=ganttChartStepArray[step].clock;
            }
            else
            {
                executionlength=ganttChartStepArray[step].clock-ganttChartStepArray[step-1].clock;
            }   
            	
    		unsigned z=executionlength*time_width;    		
    		printf("%*d", z, ganttChartStepArray[step].clock );
        }
    }
		
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
			rc=pthread_create(&thread,NULL,fcfs,(void *) NULL);	// not passing anything
			if (rc){ printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);//exit with error
            }
			
			break;
	case 2: rc=pthread_create(&thread,NULL,roundRobin,(void *) NULL);	// not passing anything
			if (rc){ printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);//exit with error
            }
	break;
	case 3: rc=pthread_create(&thread,NULL,priority_aging,(void *) NULL);	// not passing anything
			if (rc){ printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);//exit with error
            }	
	       break;
	       
	case 4: break;


}
printf("\n\n\tPress any key to continue………");
pthread_exit(NULL);

}
