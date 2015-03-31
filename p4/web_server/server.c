#include "cs537.h"
#include "request.h"
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include "defs.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//


//sx:some define
#define FIFO 0
#define SFNF 1
#define SFF	 2
#define MAXFILESIZE MAXLINE*1000
//#define T_NO 2


// CS537: Parse the new arguments too
void getargs(int *port, int *threads_no, int *buffers_size, int *schedalg, int argc, char *argv[])
{
    if (argc != 5) {
	fprintf(stderr, "Usage: %s <port> <threads> <buffers> <shedalg>\n", argv[0]);
	exit(1);
    }
    //sx_____________
    *port = atoi(argv[1]);
    if( *port <= 2000) sx_error("bad port no\n");
    *threads_no = atoi(argv[2]);
	if( *threads_no <0 ) sx_error("bad threads no\n");
    *buffers_size = atoi(argv[3]);
	if(*buffers_size<0) sx_error("bad buffer size\n");
	char schedarg[1024];
	strcpy(schedarg,argv[4]);
	if		(strcmp(schedarg,"FIFO")==0) *schedalg=0;
	else if	(strcmp(schedarg,"SFNF")==0) *schedalg=1;
	else if	(strcmp(schedarg,"SFF")==0)  *schedalg=2;
	else sx_error("bad schedalg\n");
	//sx--------------
}


int main(int argc, char *argv[])
{
	int i;
    getargs(&port, &threads_no,&buffers_size, &schedalg, argc, argv);

    // 
    // CS537: Create some threads...
    // sx______________________________
    // some initializations
    p_fdBuf	= (stBufEle *)malloc(sizeof(stBufEle)*buffers_size); 
	for(i=0;i<buffers_size;i++)
	{
		p_fdBuf[i].valid = 0;//invalid
	}
    //open buffers_size mem for fd buffer, and point to it.
    p_fdBufFirst = p_fdBuf;
	p_fdBufEnd = p_fdBuf;

	//point to the first elemetn;
	noBufEle = 0;
	produce_i = 0;
	consume_i = 0;

	//sx:threads init
	if(pthread_mutex_init(&Lock,NULL) !=0) sx_error("init lock failed\n");
	if(pthread_cond_init(&Full, NULL) !=0) sx_error("init cond full failed\n");
	if(pthread_cond_init(&Empty, NULL)!=0) sx_error("init cond empty failed\n");

	pthread_t pid,*cid;
	cid = (pthread_t *)malloc(sizeof(pthread_t)*threads_no); 
	for(i=0;i<threads_no;i++)
	{
		pthread_create(&cid[i],NULL,consumer,NULL);	
	}

    listenfd = Open_listenfd(port);
	pthread_create(&pid,NULL,producer,NULL);
/*
    while (1) {
		producer(p_fdBuf);
	// 
	// CS537: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work. However, for SFF, you may have to do a little work
	// here (e.g., a stat() on the filename) ...
	//
		consumer(p_fdBuf); 
    }
*/
	pthread_join(pid,NULL);
	for(i=0;i<threads_no;i++)
	{
		pthread_join(cid[i],NULL);	
	}
	sx_error("never reach here\n");
	return 0;
}

void *producer(void)
{
	while(1){
		clientlen = sizeof(clientaddr);
		int acpt_connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
		//printf("fd(%d)\n",acpt_connfd);
		pthread_mutex_lock(&Lock);
		while(noBufEle == buffers_size){
			printf("produecer wait..\n");
			pthread_cond_wait(&Empty,&Lock);
		}
		produce(acpt_connfd);	
		//print_buf();
		pthread_cond_signal(&Full);
		pthread_mutex_unlock(&Lock);
	}
}

void produce(int prod_connfd)
{
	while(p_fdBuf[produce_i].valid == 1){	//occupied
		produce_i = (produce_i+1)%buffers_size;
	}

	p_fdBuf[produce_i].fd = prod_connfd;
	p_fdBuf[produce_i].valid = 1; //occupied;
	p_fdBuf[produce_i].newF  = 1; //new data;
	produce_i = (produce_i +1)%buffers_size;
	noBufEle++;
	//printf("produce(%d) done\n",prod_connfd);
}

void *consumer(){
	while(1)
	{
		pthread_mutex_lock(&Lock);
		while(noBufEle == 0)	
		{
			printf("consumer wait...\n");
			pthread_cond_wait(&Full,&Lock);
		}
		char buf_rio[MAXLINE];
		int fd = consume(buf_rio);
		pthread_cond_signal(&Empty);
		pthread_mutex_unlock(&Lock);
		requestHandle(fd,buf_rio);
		Close(fd);
		//print_buf();
	}
}

int consume(char *buf_rio)
{
	int fd = consume_policy(buf_rio);
	noBufEle--;	
	//printf("consume(%d) done\n",fd);
	return fd;
}

int consume_policy(char *buf_rio)
{
	int find_fd;
	if( schedalg == 0)	//FIFO	
	{
		find_fd = p_fdBuf[consume_i].fd;
		p_fdBuf[consume_i].valid = 0;//consumed
		consume_i = (consume_i + 1)% buffers_size;
	}	
	if( schedalg == 1)	//SFNF
	{
		fill_newfdBufRio();
		int sOffset = get_smallestFileNameSizeOffset(buf_rio);	
		find_fd = p_fdBuf[sOffset].fd;
		p_fdBuf[sOffset].valid = 0;	//consumed
	}
	if( schedalg == 2)	//SFF
	{
		fill_newfdBufRio();
		int sOffset = get_smallestFileSizeOffset(buf_rio);	
		find_fd = p_fdBuf[sOffset].fd;
		p_fdBuf[sOffset].valid = 0;	//consumed
	}
	return find_fd;
}

void fill_newfdBufRio()
{
	int i;
	for(i =0 ;i<buffers_size;i++){
		if(p_fdBuf[i].valid == 1 && p_fdBuf[i].newF == 1){//occupied&&new 
			fill_fdRio(i);
		}//correct
	}	
}

void fill_fdRio(int offset)
{
   //printf("fille_fdRio...\n");
   char tempA[MAXLINE],tempB[MAXLINE];
   Rio_readinitb(&p_fdBuf[offset].rio, p_fdBuf[offset].fd);
   Rio_readlineb(&p_fdBuf[offset].rio, p_fdBuf[offset].buf, MAXLINE);
   sscanf(p_fdBuf[offset].buf, "%s %s %s", tempA, p_fdBuf[offset].file.fileName, tempB);
   p_fdBuf[offset].file.fileNameSize = 
		get_fileNameSize(p_fdBuf[offset].file.fileName);
   p_fdBuf[offset].file.fileSize = 
		get_fileSize(p_fdBuf[offset].file.fileName);	
   p_fdBuf[offset].newF = 0;	//old ele
}

int 
get_smallestFileSizeOffset(char *buf_rio)
{
	int i,smallestOffset=-1;
	off_t smallestSize=MAXFILESIZE; //MAX file Size
	for(i = 0;i< buffers_size;i++){
		if(p_fdBuf[i].valid == 1){	//occupied
			if(p_fdBuf[i].file.fileSize<smallestSize){
				smallestSize 	= p_fdBuf[i].file.fileSize;
				smallestOffset  = i;
			}
		}	
	}
	if(smallestOffset == -1) sx_error("get_smallesFNS failed\n");
	strcpy(buf_rio , p_fdBuf[smallestOffset].buf); 
	return smallestOffset;
}

int 
get_smallestFileNameSizeOffset(char *buf_rio)
{
	//printf("get_smallFNameSOffset...\n");
	int i;
	int smallestSize=MAXLINE, smallestOffset=-1;
	for(i = 0 ; i< buffers_size;i++){
		if(p_fdBuf[i].valid == 1){	//occupied
			if(p_fdBuf[i].file.fileNameSize<smallestSize){
				smallestSize 	= p_fdBuf[i].file.fileNameSize;
				smallestOffset  = i;
			}
		}	
	}
	if(smallestOffset == -1) sx_error("get_smallesFNS failed\n");
	strcpy(buf_rio , p_fdBuf[smallestOffset].buf); 
	return smallestOffset;
}

int get_fileNameSize(char *fileName)
{
	int i;
	for(i =0;fileName[i]!='\0';i++){}	
	if(i==MAXLINE) sx_error("file name too long\n");
	return i;
}

off_t get_fileSize(char *fileName)
{
	struct stat buf;
	char *fileNameReal;
	fileNameReal = strtok(fileName,"?");
	printf("fileNameReal(%s)\n",fileNameReal);
	if(stat(fileNameReal+1, &buf)<0) sx_error("stat file failed\n");
	if(buf.st_size>MAXFILESIZE)
		sx_error("file too large\n");
	return buf.st_size;
}

void print_buf()
{
	int i;
	printf("buf:");
	for(i=0;i<buffers_size;i++)
	{	
		printf("(%d)",p_fdBuf[i].fd);
	}
	printf("\n");
}
 
