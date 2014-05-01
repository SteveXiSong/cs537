#ifndef MSG_H
#define MSG_H
#include "mfs.h"

#define LOOKUP   (0)
#define STAT     (1)
#define WRITE    (2)
#define READ     (3)
#define CREAT    (4)
#define UNLINK   (5)
#define SHUT     (6)

struct msg{
  int msg_type;
  int type;
  int inum;
  int pinum;
  int block;
  int rspns;
  char buffer[4096];
  char name[60];
  MFS_Stat_t stat;
  MFS_DirEnt_t dir;

};

#endif
