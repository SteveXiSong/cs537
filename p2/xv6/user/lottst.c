#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

#define PRINT 1
#define EXTIME 20000
int dosome(int, int,int );
int main(void)
{
	//printf(1,"lottery tst...\n");
  //printf(1,"EXTIME(%d)StartTime:%d\n",EXTIME,uptime());
	int fpid;
	fpid = fork();
	if(fpid <0) exit();
	if(fpid ==0)
	{
		settickets(3);
		dosome(EXTIME,0,5);
		int i;
	struct pstat pstat;
	getpinfo(&pstat);
	for(i=0;i<NPROC;i++)
	{
		if(pstat.inuse[i] == 1)
	printf(1,"pid(%d)hticks(%d)lticks(%d)queue(%d)tickets(%d)\n",pstat.pid[i],pstat.hticks[i],pstat.lticks[i],pstat.queue[i],pstat.tickets[i]);
	}	
		exit();
	}
	else
	{
		settickets(10);
		dosome(EXTIME,1,10);
	}
	int i;
	struct pstat pstat;
	getpinfo(&pstat);
	for(i=0;i<NPROC;i++)
	{
		if(pstat.inuse[i] == 1)
	printf(1,"pid(%d)hticks(%d)lticks(%d)queue(%d)tickets(%d)\n",pstat.pid[i],pstat.hticks[i],pstat.lticks[i],pstat.queue[i],pstat.tickets[i]);
	}	
	wait();
   exit();
}


int dosome(int time, int no,int pid)
{
	int i,j;
	int time0;
	time0=uptime();
	for(i=0;i<time;i++){
		for(j=0;j<time;j++);
	}
	return 0;
}
