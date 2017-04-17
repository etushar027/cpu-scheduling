#include<stdio.h>
#include<string.h>
#include<pthread.h>
#define NUM_THREADS 5
//#include<conio.h>

#include<stdlib.h>

typedef struct process
{
int pid,st,ft,prior;
int wt,tat,at,bt;
int flag;
}pr;

int n;
pr p[10];
void accept(int);
void fcfs();
void disp_gantt();
void sjf_sort();
void priority_sort();
void round_robin(int);

void main()
{

pthread_t threads[NUM_THREADS];
int rc;long t;
rc=pthread_create(&threads[t],NULL,fcfs,(void *)&t);
printf("pthread created %#D\n");



int ch,tq;
//clrscr();
do
{
	printf("\n\t —- Scheduling Algorithms —-");
	printf("\n\t1. FCFS\n \t2. SJF \n \t3. Priority Based\n \t4. Round Robin\n \t5. Exit");
	printf("\n\t Enter your choice: ");
	scanf("%d",&ch);
	switch(ch)
{
	case 1: accept(ch);
	fcfs();
	break;

	case 2: accept(ch);
	sjf_sort();
fcfs();
break;

case 3: accept(ch);
priority_sort();
fcfs();
break;

case 4: printf("\n\tEnter the Time Quantum: ");
scanf("%d",&tq);
accept(ch);
round_robin(tq);break;

case 5: break;
}
printf("\n\n\tPress any key to continue………");
//getch();
//clrscr();
}
while(ch!=5);
//getch();
}

void accept(int ch)
{
int i;
printf("\n\tHow many processes: ");
scanf("%d",&n);
printf("\tEnter the values\n");
if(ch==1 || ch==2)
{
printf("\n\tArrival Time and Burst Time\n");
for(i=0;i<n;i++)
{
printf("\tEnter for Process %d :",i);
scanf("%d%d",&p[i].at,&p[i].bt);
p[i].pid=i;
}
}
if(ch==3)
{
printf("\n\tArrival Time , Burst Time and Priority\n");
for(i=0;i<n;i++)
{
printf("\tEnter for Process %d :",i);
scanf("%d%d%d",&p[i].at,&p[i].bt,&p[i].prior);
p[i].pid=i;
}
}
if(ch==4)
{
printf("\n\tArrival Time and Burst Time\n");
for(i=0;i<n;i++)
{
printf("\tEnter for Process %d :",i);
scanf("%d%d",&p[i].at,&p[i].bt);
p[i].pid=i;
}
}
}

void fcfs()
{
int i;
float avgtat=0,avgwt=0;
for(i=0;i<n;i++)
{
if(i==0)
{
p[i].st=i;
p[i].wt=p[i].st-p[i].at;
p[i].tat=p[i].wt+p[i].bt;
p[i].ft=p[i].st+p[i].bt;
}
else
{
p[i].st=p[i-1].ft;
p[i].wt=p[i].st-p[i].at;
p[i].tat=p[i].wt+p[i].bt;
p[i].ft=p[i].st+p[i].bt;
}
avgtat=avgtat+p[i].tat;
avgwt=avgwt+p[i].wt;
}
avgtat=avgtat/n;
avgwt=avgwt/n;
//disp_gantt(n);
printf("\n\t** Average Turn Around Time:%f **",avgtat);
printf("\n\t** Average waiting time:%f **",avgwt);
}

/*void disp_gantt()
{
int i,p;
printf("\n\tGANTT CHART\n ");
for(i=0;i<n;i++)
{
printf("\t p %d",p[i].pid);
}
printf("\n");
for(i=0;i<n;i++)
{
printf("\t %d",p[i].st);
}
printf("\t %d",p[i-1].ft);
printf("\n");
} */

void sjf_sort()
{
int i,j;
pr temp;
for(i=0;i<n;i++)
{
for(j=0;j<n-i-1;j++)
{
if(p[j].bt>p[j+1].bt)
{
temp=p[j];
p[j]=p[j+1];
p[j+1]=temp;

}
}
}
}

void priority_sort()
{
int i,j;
pr temp;
for(i=0;i<n;i++)
{
for(j=0;j<n-i-1;j++)
{
if(p[j].prior>p[j+1].prior)
{
temp=p[j];
p[j]=p[j+1];
p[j+1]=temp;

}
}
}
}

void round_robin(int tq)
{
int allterminate=0,cnt=0,i;
int gt[30],ps[30],temp=0,pcnt=0;
float avgtat=0;
printf("\n\tGANTT CHART\n\n");
while(allterminate<n)
{
for(i=0;i<n;i++)
{
if(p[i].flag==0) //process incomplete
{
p[i].st=temp;
cnt++;
if(p[i].bt>=tq)//burst time > time slice
{

p[i].bt=p[i].bt-tq;
temp=temp+tq;
gt[cnt]=temp;
ps[pcnt]=p[i].pid;
pcnt++;
}
else
{ //burst time < time slice
temp=temp+p[i].bt;
gt[cnt]=temp;
p[i].bt=0;
ps[pcnt]=p[i].pid;
pcnt++;
}

if(p[i].bt==0)
{
allterminate++;
p[i].flag=1;
p[i].ft=temp;
}

if(p[i].flag==1)
{
p[i].tat=p[i].ft;
avgtat=avgtat+p[i].tat;
}
}//if

}//for

}//while
for(i=0;i<pcnt;i++)
{
printf("\t P%d",ps[i]);
}
printf("\n\t0 ");
for(i=1;i<=cnt;i++)
{
printf("\t %d ",gt[i]);
}
avgtat=avgtat/n;
printf("\n\t** Average Turn Around Time:%f **",avgtat);
}
