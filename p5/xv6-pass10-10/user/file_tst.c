#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"

int main()
{
	int fd;
	char buf[100];
	fd=open("tstfile",O_CREATE|O_CHECKED|O_RDWR);
	printf(1,"fd(%d)\n",fd);
	if(fd<0)
		printf(1,"open failed\n");
		
    	if(write(fd, "aaaaaaaaaa", 10) != 10){
      		printf(1, "error: write aa  new file failed\n");
      		exit();
	}
	struct stat st;
	fstat(fd,&st);
	printf(1,"%s\n",st.checksum);
	close(fd);
	fd=open("tstfile",O_CHECKED|O_RDWR);
	if(read(fd,buf,10)!=10)
		printf(1,"read fail\n");
	printf(1,"read end\n");
	printf(1,"%s\n",buf);
	//-----------------------------
	int fd1=open("tstfile1",O_CREATE|O_RDWR);
	printf(1,"fd(%d)\n",fd1);
	if(fd1<0);
	//struct stat st;
	fstat(fd1,&st);
	printf(1,"%s\n",st.checksum);
		
    	if(write(fd1, "aaaaaaaaaa", 10) != 10){
      		printf(1, "error: write aa  new file failed\n");
      		exit();
	}
	if(read(fd1,buf,10)!=10)
		printf(1,"read fail\n");
	printf(1,"read end\n");
	printf(1,"%s\n",buf);
	exit();
}


