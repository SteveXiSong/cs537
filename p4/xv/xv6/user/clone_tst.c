#include "types.h"
#include "stat.h"
#include "user.h"
#define ONEK 1024
#define PGSIZE 4*ONEK
void do_sth(void *);
void do_sthelse(int);
/*
int pseudo_thread_create(void(*fcn)(void*),void*);
int pseudo_thread_join(void);
*/
lock_t Mlock;
cond_t cv1;
cond_t cv2;
int main()
{
	int i;
	void (*fcn)(void*);	
	fcn = do_sth;
	i=200;
	int pid = getpid();
	lock_init(&Mlock);
	int tid = thread_create(fcn,(void *)&i);
	if(tid == 0)	//child
	{
		i++;
		printf(1,"fake proc child\n");
		do_sth((void *)&i);
	}
	else //parent
	{
		
		lock_acquire(&Mlock);	
		int k=0;
		while(k<1000*1000)k++;
		i++;
		printf(1,"pid(%d)parent\n",pid);
		do_sthelse(i);
		lock_release(&Mlock);
	}
	if(thread_join() == -1)printf(1,"no child thread\n");;	
	exit();
}

void do_sth(void *i)
{
	lock_acquire(&Mlock);	
	(*(int *)i) ++;
	printf(1,"do_sth:i(%d)&cv1(%p)&Mlock(%p)\n",*(int *)i,&cv1,&Mlock);
	//cv_wait(&cv1,&Mlock);
	cv_signal(&cv2);
	lock_release(&Mlock);
	exit();
}
void do_sthelse(int i)
{
	cv_wait(&cv2,&Mlock);
	//cv_signal(&cv1);
	printf(1,"do_sthelse:i(%d)&cv2(%p)\n",i,&cv2);
}


