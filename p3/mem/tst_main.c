#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mem.h"
#include "tree.h"

void *mHeadAddr= NULL;
int main(void)
{
	int sizeOfRegion = getpagesize();
	Mem_Init(sizeOfRegion);	
	Mem_Alloc(0,0);
	printf("-mHeadAddr(%p)\n-sizeOfRegion(%d)\n",mHeadAddr,sizeOfRegion);

	return EXIT_SUCCESS;
}

