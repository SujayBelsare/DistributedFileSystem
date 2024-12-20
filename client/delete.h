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

int processDeleteResponse(Message *response);

#endif