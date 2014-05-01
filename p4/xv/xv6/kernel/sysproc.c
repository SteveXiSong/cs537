#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"
#include "spinlock.h"

//sx
int
sys_clone(void)
{
	void (*fcn)(void*);
	void *arg; void *stack;	
	if(	argptr(0,(char **)&fcn, sizeof(void *))<0
	  ||argptr(1,(char **)&arg, sizeof(void *))<0
	  ||argptr(2,(char **)&stack,sizeof(void *))<0)
		return -1;
	
	//cprintf("sys_clone:stack%PGSIZE(%d)\n",(uint)stack%PGSIZE);
	if((uint)stack%PGSIZE != 0) return -1;//check page align
	if((uint)stack+PGSIZE > proc->sz) return -1;
	//cprintf("*********sys_clone fetch arg done\n");
	return clone(fcn,arg,stack);
}

int
sys_join(void)
{
	void **stack;
	if(argptr(0,(char **)&stack,sizeof(void *))<0)
		return -1;
	//cprintf("sys_join:stack(%p)\n",stack);
	return join(stack);
}

int
sys_t_sleep(void)
{
	void *chan;
	lock_t *p_lk;
	if(	argptr(0,(char **)&chan, sizeof(void *))<0
	  	||argptr(1,(char **)&p_lk, sizeof(void *))<0
	)
		return -1;
	else
		t_sleep(chan,p_lk);
	return 0;
}
	
int
sys_t_wakeup(void)
{
	void *chan;
	if(	argptr(0,(char **)&chan, sizeof(void *))<0)
		return -1;
	else
		t_wakeup(chan);
	return 0;
}

//*********************************

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
  if(proc->thread.is_thread == 1)
  	addr = proc->parent->sz;//sx:t size
  else addr = proc->sz;
  //addr = proc->parent->sz;//sx:t size
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
