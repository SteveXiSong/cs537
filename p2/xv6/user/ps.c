#include "types.h"
#include "pstat.h"
#include "stat.h"
#include "user.h"

int main(void)
{
	struct pstat pstat;
	int i;
	getpinfo(&pstat);
	for(i=0;i<NPROC;i++)
		if(pstat.pid[i] != 0)
		printf(1,"pid(%d) inuse(%d) hticks(%d) lticks(%d)\n",pstat.pid[i],pstat.inuse[i],pstat.hticks[i],pstat.lticks[i]);
	exit();
}
