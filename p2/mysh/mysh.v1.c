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
	cmd_buf[0]= '\0';
	in_buf[0]='\0';
	strcpy(error_message,"An error has occured\n");
	if((homenv=getenv("HOME"))==NULL) unexp_error(0);

	//left blank to add sth------------	
	//
	//start the main part------
	
loop_main:
	fflush(stdin); 	//flush the stdin buf
	printf("mysh> ");	//print prompt
	fgets(cmd_buf,MAXCMDBYTES,stdin);	//get the cmd from keyboard input
	// go into the main function, first parse the cmd into several parts and store in the struct CmdIn, and explain the cmd. finally execute it.
	if(parseNmain()==1) goto loop_main;	//nothing in
	explainCmd();
goto loop_main;
  return 0;

}

//to explain the cmd 
int explainCmd(void)
{
//	printf("explainCmd is runing\n");
	is_pipe();
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

int clr_enter(char *cmd_buf)
{
	int i=0;
	while(cmd_buf[i]!='\0')
	{
		if(cmd_buf[i] == '\r') {cmd_buf[i]='\0';break;} //clr enter	
		if(cmd_buf[i] == '\n') {cmd_buf[i]=' ';} 	//clr tab
		i++;
	}
	return 0;
}

int parseNmain()
{
 //	printf("parseNmain running...\n"); 
	CmdIn.argc =0; //the number of the opts of the cmd
	int i;
	clr_enter(in_buf); //clear all the \r and \n
	is_pipe();
  	if(cmd_buf[0]!='\0'&&cmd_buf[0]!='\r')
 	{
		if((CmdIn.argv[0]=strtok(cmd_buf," ")) == NULL) 
			return 0;  //read in the first cmd
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
  return 1;
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
	CmdPipe.flag=0;
  	if(in_buf[0]!='\0'&&in_buf[0]!='\r')
 	{
		if((cmd_buf=strtok(in_buf,">")) == NULL) 
			return 0;  //read in the first cmd
		CmdPipe.flag++;
		while(1) 
		{
			if((CmdPipe.output=strtok(NULL,">"))==NULL){
				if(CmdPipe.flag>2) unexp_error(1); //store the number of the cmd	
				break;
			}
			CmdPipe.flag++;
		}	//end of parse cmd 
	}	
	printf("CmdIn:%s\nCmdPipe:%s\n",cmd_buf,CmdPipe.output);
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
