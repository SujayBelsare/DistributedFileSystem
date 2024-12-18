#ifndef COPY_H
#define COPY_H

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


int processCopyResponse(Message* response);
int processListResponse(Message* response);

extern bool exchange_messages(const char *ip, int port, Message *request, Message *response);

#endif