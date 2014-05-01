#ifndef _TYPES_H_
#define _TYPES_H_

// Type definitions

typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef uint pde_t;
//sx:thread**********
typedef struct _lock_t{
	unsigned int lock;
}lock_t;

typedef unsigned int cond_t;
#define UNLOCKED 0
#define LOCKED 1
//******************
	
#ifndef NULL
#define NULL (0)
#endif

#endif //_TYPES_H_
