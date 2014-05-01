#ifndef _TREE_H_
#define _TREE_H_
//#define EXIT_SUCCESS 	0
//#define EXIT_FAIL	-1

#define handle_error(msg) \
	do{perror(msg);exit(EXIT_FAILURE);}while(0)
struct tree_t
{
	struct tree_t * father;
	int level;	//top =>0, down 1
	int addr;
	int is_free;		
	struct tree_t *lchild;
	struct tree_t *rchild;
};





#endif
