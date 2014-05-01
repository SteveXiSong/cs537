#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"
#include "pstat.h"
//sx
int sys_settickets(void)
{
	int tickets;
	if(argint(0, &tickets) < 0)
    		return -1;
//	cprintf("tickets:%d\n",tickets);
	if(settickets(tickets)== -1) return -1;
	return 0;
}
//sx
int sys_getpinfo(void)
{
	int sz=sizeof(struct pstat );
	char *arg;
	if(argptr(0, &arg,sz) < 0)
    		return -1;
	struct pstat *p_pstat = (struct pstat *) arg;
	getpinfo(p_pstat);	
	//cprintf("PID    inuse    hticks    lticks      usedTimes      queue\n");
	//int i= proc->pid;
	//cprintf("%d      %d           %d          %d      %d     %d\n",pstat.pid[i],pstat.inuse[i],pstat.hticks[i],pstat.lticks[i],pstat.usedTimes[i],pstat.queue[i]);
	//cprintf("(%d)endtime:%d\n",pstat.pid[i],sys_uptime());
	return 0;
}


int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;
  
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since boot.
int
sys_uptime(void)
{
  uint xticks;
  
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
