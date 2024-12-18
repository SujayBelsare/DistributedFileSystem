#ifndef _GET_DATA_H_
#define _GET_DATA_H_

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

int processGetResponse(Message* response);
extern bool exchange_messages(const char *ip, int port, Message *request, Message *response);

#endif