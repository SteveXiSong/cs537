#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "mysh.h"
//*****************************
#define MAXCMDBYTES 512




//the main function***********************************
int main(int argc, char *argv[])
{
	strcpy(cmd_buf,"\0");
	//strcpy(pipe_buffer,"\0");
	strcpy(in_buf , "\0");
	strcpy(error_message,"An error has occured\n");
	if((homenv=getenv("HOME"))==NULL) unexp_error(0); //pwd needs

	//left blank to add sth------------	
	//
	//start the main part------
	
    while(1){
	fflush(stdin); 	//flush the stdin buf
	printf("mysh> ");	//print prompt
	fgets(in_buf,MAXCMDBYTES,stdin);	//get the cmd from keyboard input
	// go into the main function, first parse the cmd into several parts and store in the struct CmdIn, and explain the cmd. finally execute it.
	printf("input: %s(%d)\n",in_buf,*in_buf);
	clr_enter(in_buf); //clear all the \r and \n
	if(is_pipe()==1) continue;
	if(parseNmain()==1) continue;	//nothing in
	explainCmd();
    }
  return 0;

}

//to explain the cmd 
int explainCmd(void)
{
	printf("explainCmd is runing\n");
	//is_pipe();
	//build in cmd------------------
	if(strcmp(CmdIn.argv[0],"exit")==0){
		sh_exit();
		return 0;
	}
	if(strcmp(CmdIn.argv[0],"pwd")==0){
		sh_pwd();
		return 0;
	}
	if(strcmp(CmdIn.argv[0],"cd")==0){
		sh_cd();
		return 0;
	}
	if(strcmp(CmdIn.argv[0],"wait")==0){
		sh_wait();
		return 0;
	}
	//need to fork cmd----------------
	if(strcmp(CmdIn.argv[0],"ls")==0){
		sh_ls();
		return 0;
	}	

	printf("no matched cmd\n");	
	unexp_error(0);
	return 0;	
}

int clr_enter(char *buf)
{
	int i=0;
	while(buf[i]!='\0')
	{
		if(buf[i] == '\r') {buf[i]='\0';break;} //clr enter	
		if(buf[i] == '\n') {buf[i]=' ';} 	//clr tab
		i++;
	}
	return 0;
}

int parseNmain()
{
	printf("parseNmain running...\n"); 
	CmdIn.argc =0; //the number of the opts of the cmd
	int i;
	clr_enter(cmd_buf); //clear all the \r and \n
	//printf("cmd_buf:%s(%d)\n",cmd_buf,*cmd_buf);
  //	if(*cmd_buf!='\0'&&*cmd_buf!='\r'&&*cmd_buf!='\n'&&cmd_buf!= NULL)
  	if(cmd_buf!=NULL&&*cmd_buf!=10)
 	{
		if((CmdIn.argv[0]=strtok(cmd_buf," ")) == NULL) 
			return 1;  //read in the first cmd
		i=1;
		//cmd parse, read in the other opts of the cmd
		while(1) 
		{
			if((CmdIn.argv[i]=strtok(NULL," "))==NULL){
				CmdIn.argc=i; //store the number of the cmd
				break;
			}
			i++;
		}	//end of parse cmd 
#ifdef PRINT_ON
		for(i=0;i<CmdIn.argc;i++)
		{
			printf("CmdIn.cmdargv[%u]:%s\n",i,CmdIn.argv[i]);
		}
#endif
		return 0;
  	}
	else return 1;
  return 0;
}
//report error and exit(0) as task required
int unexp_error(int n)
{
	printf("(%d)",n);
	write(STDERR_FILENO,error_message, strlen(error_message));
	exit(1);
}

int is_pipe()
{
	printf("is_pipe running..\n");
	CmdPipe.flag=0;
	int i =0;
  	if(*in_buf!='\0'&&*in_buf!=10)
 	{
		
		if((strcpy(cmd_buf,strtok(in_buf,">")))==NULL)
			return 1;  //read in the first cmd
		CmdPipe.flag++; //here flag should be 1
		while(1) 
		{
			if((pipe_buffer[i]=strtok(NULL,">"))==NULL)
			//if((strcpy(pipe_buffer,strtok(NULL,">")))==NULL)
			{
				if(CmdPipe.flag>2) unexp_error(1); //store the number of the cmd	
				break;	//detected all >
			}
			CmdPipe.flag++; //if already a second >, here is 2
			printf("pipe_buffer: %s(%d)\n",pipe_buffer[i],*pipe_buffer[i]);
			i++;
		}	//end of parse cmd 
	}	
	else	return 1;
	printf("CmdIn:%s(%d)\nPipeBuf:%s\n",cmd_buf,cmd_buf[0],pipe_buf);
	printf("CmdIn:%s(%d)\nPipeBuf:%s\n",cmd_buf,cmd_buf[0],pipe_buffer[0]);
	return 0;
}
/*
int is_pipe()
{
	printf("is_pipe is running..\n");
	int i,j;
	CmdPipe.flag=0;
	int argv_flag=0;
	for(i=0; i<CmdIn.argc;i++)
	{
		if(argv_flag ==1) unexp_error(1);
		for(j=0;CmdIn.argv[i][j]!='\0';j++)
		{
			if(CmdIn.argv[i][j] == '>')
			{
				if(++CmdPipe.flag>1) unexp_error(2);   //to sum the times of detected >, if there are more than 1 >, then error
				CmdIn.argv[i][j] = '\0';
				continue;
			}
			if(CmdPipe.flag == 1) 
			{
				CmdPipe.output = &CmdIn.argv[i][j];
				argv_flag=1;
				break;
			}
		}
	}
	printf("CmdPipe.output:(%d) %s\n",CmdPipe.flag,CmdPipe.output);
	return 0;

}
*/
