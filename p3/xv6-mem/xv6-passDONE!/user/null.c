#include "types.h"
#include "stat.h"
#include "user.h"

#define _1K 1024

int main(void)
{
	int i;
 
        int variable = 0;
        printf(1,"Addr_stack(%dK=%d)\n",((int)&variable)/1024,&variable);
	////////////
	printf(1,"Open 3K stack space\n");
	char vari[4*_1K] = {0};
        printf(1,"Addr_stack(%dK)\n",((int)vari)/_1K);
	char vari_1[4*_1K] = {0};
        printf(1,"Addr_stack(%dK)\n",((int)vari_1)/_1K);
	
	/////////////////
	/*
        int *ptr_heap_1 = (int *)malloc(10);
        printf(1,"Addr_heap(%dK=%d)\n",((int)ptr_heap_1)/1024,ptr_heap_1);
	*//////////////
	char *ptr_heap[160];
	for(i = 0; ptr_heap[i] == 0;i++)
	{
        	ptr_heap[i] = (char *)malloc(4*_1K);
        	printf(1,"Addr_heap[%d](%dK=%d)\n",i,((int)ptr_heap[i])/1024,ptr_heap[i]);
		if(ptr_heap[i] == 0) break;
        }
	char *ptr_heap_sub[100];
  	for(i=0;;i++)
	{
		ptr_heap_sub[i] = (char *)malloc(4*_1K);
		printf(1, "Addr_heap_sub[%d](%dK)\n",((int)ptr_heap_sub[i])/_1K);
		if(ptr_heap_sub[i] == 0) break;
	}
//////////////////////////
        int *ptr =NULL;
        printf(1,"NULL(%d)\n",ptr);
        printf(1,"NULL(%d)\n",*ptr);
////////////////////////////
        exit();
}
        
