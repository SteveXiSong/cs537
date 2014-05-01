#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mfs.h"
#include "udp.h"
#include "msg.h"

#define TIMESPAN (5)

int sd;
struct sockaddr_in saddr;
struct msg msg;

int rcv_msg(){
  fd_set r;
  struct timeval t;
  FD_ZERO(&r);
  FD_SET(sd, &r);
  t.tv_sec  = TIMESPAN;
  t.tv_usec = 0;
  return  select(sd+1, &r, NULL, NULL, &t);
}


int MFS_Init(char *hostname, int port){
  sd = UDP_Open(0);
  assert(sd > -1);

  int rc = UDP_FillSockAddr(&saddr, hostname, port);
  assert(rc == 0);
  printf("CLIENT:: about to send message (%d)\n", rc);
  return 0;
}

int MFS_Lookup(int pinum, char* name){
  /************ message server will process **********/
  msg.msg_type = LOOKUP;
  msg.pinum = pinum;
  strcpy(msg.name, name);

  // sent message
  int rc = UDP_Write(sd, &saddr, (char*)&msg, sizeof(msg));
  printf("lookup message sent\n");
  // time out options
  rc = rcv_msg();

  // recevice stuff from server
  if(rc > 0){
    struct sockaddr_in raddr;
    UDP_Read(sd, &raddr, (char*)&msg, sizeof(msg));
    // check the return inum item
    if(msg.inum > 0)
	return msg.inum;
    else
	return -1;
  }
  else{
    printf("timed out\n");
    return -1;  // timed out
  }
}

int MFS_Stat(int inum, MFS_Stat_t *m){
  /************ message server will process **********/
  msg.msg_type = STAT;
  msg.inum = inum;

  // sent message
  int rc = UDP_Write(sd, &saddr, (char*)&msg, sizeof(msg));
  printf("stat message sent\n");
  // time out options
  rc = rcv_msg();

  if(rc > 0){
    struct sockaddr_in raddr;
    rc = UDP_Read(sd, &raddr, (char*) &msg, sizeof(msg));
    if(msg.rspns < 0)
	return -1;
    else{
	m->type = msg.stat.type;
	m->size = msg.stat.size;
	return 0;
    }

  }
  else
    return -1;
}

int MFS_Write(int inum, char *buffer, int block){
  /************ message server will process **********/
  msg.msg_type = WRITE;
  msg.inum = inum;
  strcpy(msg.buffer, buffer);
  msg.block = block;

  // send message
  int rc = UDP_Write(sd, &saddr, (char*)&msg, sizeof(msg));
  rc = rcv_msg();

  if(rc > 0){
    struct sockaddr_in raddr;
    UDP_Read(sd, &raddr, (char*)&msg, sizeof(msg));

    return msg.rspns;
  }

  else
    return -1;  // timed out
}

int MFS_Read(int inum, char *buffer, int block){
  /************ message server will process **********/
  msg.msg_type = READ;
  msg.inum = inum;
  msg.block = block;
  // send message
  int rc = UDP_Write(sd, &saddr, (char*)&msg, sizeof(msg));
  // time out options
  rc = rcv_msg();

  if(rc > 0){
    struct sockaddr_in raddr;
    UDP_Read(sd, &raddr, (char*) &msg, sizeof(msg));

    if(msg.rspns < 0)
	return -1;
    else{
	strcpy(buffer, msg.buffer);
	return 0;
    }
  }
  else
    return -1;  // timed out

}

int MFS_Creat(int pinum, int type, char *name){
  /************ message server will process **********/
  msg.msg_type = CREAT;
  msg.pinum = pinum;
  msg.type = type;
  strcpy(msg.name, name);

  // send message
  int rc = UDP_Write(sd, &saddr, (char*)&msg, sizeof(msg));

  // time out options
  rc = rcv_msg();
  if(rc > 0){
    struct sockaddr_in raddr;
    UDP_Read(sd, &raddr, (char*)&msg, sizeof(msg));   

    return msg.rspns;
  }
  else
    return -1; // timed out
}

int MFS_Unlink(int pinum, char *name){
  /************ message server will process **********/
  msg.msg_type = UNLINK;
  msg.pinum = pinum;
  strcpy(msg.name, name);

  // send message
  int rc = UDP_Write(sd, &saddr, (char*)&msg, sizeof(msg));
  // time out options
  rc = rcv_msg();

  if(rc > 0){
    struct sockaddr_in raddr;
    // server side will send -1 or 0 according to result
    UDP_Read(sd, &raddr, (char*)&msg, sizeof(msg));   
    return msg.rspns;
  }
  else
    return -1; // timed out
}

int MFS_Shutdown(){
  /************ message server will process **********/
  msg.msg_type = SHUT;
  printf("shut: msg_type: %d\n", msg.msg_type);
  // send message
  int rc = UDP_Write(sd, &saddr, (char*)&msg, sizeof(msg));
  // time out options
  rc = rcv_msg();
  if(rc > 0){
    struct sockaddr_in raddr;
    // server side will send -1 or 0 according to result
    rc = UDP_Read(sd, &raddr, (char*)&msg, sizeof(msg));   
    printf("shut:msg_rs: %d\n", msg.rspns);
    return msg.rspns;
  }
  else
    return -1; // timed out
}

