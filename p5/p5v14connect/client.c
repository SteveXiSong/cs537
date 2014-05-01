#include <stdio.h>
#include "udp.h"
#include "msg.h"

#define BUFFER_SIZE (4096)
char buffer[BUFFER_SIZE];

int
main(int argc, char *argv[])
{
    int sd = UDP_Open(-1);
    assert(sd > -1);

    struct sockaddr_in saddr;
    int rc = UDP_FillSockAddr(&saddr, "mumble-06.cs.wisc.edu", 10000);
    assert(rc == 0);

    printf("CLIENT:: about to send message (%d)\n", rc);
    struct msg message;
    message.msg_type = atoi(argv[1]);
    message.type = atoi(argv[2]);
    message.inum = atoi(argv[3]);
    message.pinum = atoi(argv[4]);
    message.block = atoi(argv[5]);

    rc = UDP_Write(sd, &saddr, (char*)& message, sizeof(message));
    // printf("CLIENT:: sent message (%d)\n", rc);
    if (rc > 0) {
      struct sockaddr_in raddr;
      int rc = UDP_Read(sd, &raddr, (char*)& message, sizeof(message));
      printf("CLIENT:: read %d bytes (message: '%s')\n", rc, message.rspns);
    }

    return 0;
}


