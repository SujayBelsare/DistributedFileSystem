#ifndef DELETE_H
#define DELETE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include "client.h"
#include <sys/time.h>
#include <pthread.h>

#include "client.h"

extern int connect_to_server(const char *ip, int port);
int processDeleteResponse(Message *response, const char* nmIP, int nmPort);
extern bool exchange_messages(const char *ip, int port, Message *request, Message *response);

#endif