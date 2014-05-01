#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mfs.h"
#include "udp.h"
#include "msg.h"

int
main(int argc, char *argv[])
{
    MFS_Init("mumble-05.cs.wisc.edu", atoi(argv[1]));
  MFS_Lookup(1, "aa");

  MFS_Stat_t stat;
  stat.type = 1;
  stat.size = 10;
  MFS_Stat(10, &stat);
  char buffer[4096]; 

  MFS_Write(11, buffer, 3);
  MFS_Read(12, buffer, 4);
  MFS_Creat(13, 1, buffer);
  MFS_Unlink(14, buffer);

  return  0;
}


