#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mem.h"
#include "tree.h"
#include "def.h"
//void *mHeadAddr=NULL;

int Mem_Init(int sizeOfRegion)
{
	// open the /dev/zero device
 	int fd = open("/dev/zero", O_RDWR);
	// sizeOfRegion (in bytes) needs to be evenly divisible by the page size
	mHeadAddr = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (mHeadAddr == MAP_FAILED) handle_error("mmap"); 
	// close the device (don't worry, mapping should be unaffected)
	close(fd);
	return EXIT_SUCCESS;
}
