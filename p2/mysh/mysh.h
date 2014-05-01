#ifndef _MYSH_H_
#define _MYSH_H_

#define PRINT_ON 1

//belows are for defination and declares*****************
/*static char *cmds[]={
    "cd","cp","chmod",
    "exit",
    "ls",
    "fork",
    "pwd",
    };
 */                                   
char error_message[30];
char in_buf[512]; 
char cmd_buf[512];
char *pipe_buf;
char *pipe_buffer[2];
char pipe_outfile[512];
int  python_flag;

struct cmd_in{
    	char *argv[512];    //store the cmd store the opts of the cmd
	int argc;
   }CmdIn;
struct cmd_pipe{
	int flag;
	char *output;
}CmdPipe;

char *homenv;

//other declares----------------------------
int parseNmain();
int explainCmd(void);
int is_pipe();
int clr_enter();
int is_python();
//add sh cmd below------------------------------
int sh_exit(void);
int sh_pwd();
int sh_cd();
int sh_wait();
int sh_ls();
int sh_forkNexc(void);
int sh_python();

int unexp_error(int n);
//------------------------------
#endif
