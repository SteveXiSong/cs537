#include "mysh.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>


int sh_fork()
{
	//int rc = fork();
	return 0;
}

int sh_pwd()
{	
	char buf[512];
	if(getcwd(buf,512)==NULL){fprintf(stderr, "getcwd failed\n"); exit(1);}
	printf("%s\n",buf);	

	return 0;
}

int sh_wait()
{
	pid_t pid;
//	printf("wait() is running...\n");
	if((pid=wait(NULL))== -1)unexp_error(0);
//	printf("wait for pid(%d) suc\n", pid);
	return 0;
}

int sh_exit(void)
{
     //printf("exit(0) is going to run..\n");
     exit(0);
 }

int sh_cd(void)
{	
	if(CmdIn.argc==1)
	{
		if(chdir(homenv) == -1) unexp_error(0);
	}
	if(CmdIn.argc>1)
	{
		if(chdir(CmdIn.argv[1]) == -1) unexp_error(0);
	}
	printf("current dir is:\n");
	sh_pwd();
	return 0;
}

int sh_ls(void)
{
	sh_forkNexc();	
	return 0;
}

int sh_forkNexc(void)
{
	int rc = fork();
	if(rc<0) unexp_error(0);
	else if(rc ==0)
	{
		if(CmdPipe.flag==1)	//means the pipe is on
		{
			int close_rc= close(STDOUT_FILENO);
			if(close_rc<0){
				perror("close");
				exit(1);
			}
			int fd=open(pipe_buffer[0], O_RDWR|O_TRUNC|O_CREAT,S_IRWXU);
			if(fd<0){
				perror("open");
				exit(1);
			}
			
		}
		else if(CmdPipe.flag!=0) unexp_error(0);
		//printf("child(%d)\n", (int)getpid());
		if(python_flag == 1)
		{
			execvp("python",CmdIn.argv);	
			unexp_error(0);
		}		
		else execvp(CmdIn.argv[0],CmdIn.argv);
		//should not run-----
		unexp_error(0);
	}
	else //parent proc
	{
		//here if it is BACKGROUND JOBS, you should not run wait() to wait for the child proc
		sh_wait();
		//printf("parent(%d)\nfork suc\n",(int)getpid());
		
	}
	return 0;
}

int sh_python()
{
	sh_forkNexc();
	return 0; 
}
