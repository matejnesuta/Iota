#ifndef __UDP_MODE_H__
#define __UDP_MODE_H__

#include <netdb.h>

#define UDP_BUFSIZE 1024

void prepareUDPErrResponse(char (*)[UDP_BUFSIZE], char*);
int udpMode(struct sockaddr_in);

#endif