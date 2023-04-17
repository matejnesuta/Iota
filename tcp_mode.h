#ifndef __TCP_MODE_H__
#define __TCP_MODE_H__

#include <netdb.h>

#define MAX_CLIENTS 50

int caseInsensitiveStrcmp(char*, char*);
void closeTCPConnection(int, int*, int*);
int tcpMode(struct sockaddr_in);

#endif