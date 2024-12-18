#ifndef _WRITE_H__
#define _WRITE_H__

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

int processWriteResponse(Message* response, char* content, int write_content);
extern bool exchange_messages(const char *ip, int port, Message *request, Message *response);


#endif