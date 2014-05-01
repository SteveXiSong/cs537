#include "types.h"
#include "stat.h"
#include "user.h"
#include "x86.h"
#define PGSIZE (4096)

int thread_create(void (*fcn)(void *),void *arg)
{
	void *stack;
	int offset;
	offset = (int)sbrk(0);
	if(offset%PGSIZE != 0){
		sbrk(PGSIZE-offset%PGSIZE);	
	}
	stack = malloc(PGSIZE);
	if((uint)stack%PGSIZE != 0){
		stack-= (uint)stack%PGSIZE;
	}
	//printf(1,"t_clone:stack%PGSIZE(%d)\n",(uint)stack%PGSIZE);
	int tid = clone(fcn,arg,stack);
	//printf(1,"arg(%d)tid(%d)\n",*(int*)arg,tid);
	return tid;
}
int thread_join(void)
{
	void *stack;
	int tid = join(&stack);	
	free(stack);
	return tid;
}

void lock_acquire(lock_t * lk)
{
//	printf(1,"lock(%p)\n",lk);
	while(xchg(&lk->lock,LOCKED)!=0);
}
void lock_release(lock_t * lk)
{
//	printf(1,"unlock(%p)\n",lk);
	if(lk->lock == LOCKED)
		xchg(&lk->lock,UNLOCKED);
}
void lock_init(lock_t *lk)
{
	lk->lock = UNLOCKED;//init it 
}
void cv_wait(cond_t *cv, lock_t *lk)
{
	//printf(1,"wait(%p)\n",cv);
	t_sleep((void*)cv,lk);
};
void cv_signal(cond_t *cv)
{
	//printf(1,"signal(%p)\n",cv);
	t_wakeup((void *)cv);
};
