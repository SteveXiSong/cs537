#ifndef _DEFS_H_
#define _DEFS_H_

//sx:some global define
struct sockaddr_in clientaddr;
int listenfd, connfd, port, clientlen;
int threads_no, buffers_size, schedalg;

typedef struct _stFile{
	char fileName[MAXLINE];
	int fileNameSize;
	off_t fileSize;
} stFile;

typedef struct _stBufEle{
	int fd;
	stFile file;
	rio_t rio;
	char buf[MAXLINE];
	int valid;
	int newF;
} stBufEle;


stBufEle *p_fdBuf,*p_fdBufFirst, *p_fdBufEnd;
int consume_i;
int produce_i;

int noBufEle;

pthread_mutex_t Lock;
pthread_cond_t	Full, Empty;

void *producer();
void *consumer();
void produce(int);
int consume(char *);
int consume_policy(char *);
int get_fileNameSize(char *);
off_t get_fileSize(char *);
int get_smallestFileNameSizeOffset(char *);
int get_smallestFileSizeOffset(char *);
void fill_newfdBufRio(void);
void fill_fdRio(int);
void print_buf();

struct stat fileBuf;

#endif
