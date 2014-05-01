#include "types.h"
#include "user.h"
#include "fcntl.h"


int main(void)
{
	printf(1,"system call info is: %d\n",getsyscallinfo());
	exit();
}
