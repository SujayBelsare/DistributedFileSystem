#ifndef _WRITE_H__
#define _WRITE_H__

#include "clientheader.h"

#define WRITE_STATUS_CHECK_INTERVAL 2

typedef struct
{
    int request_id;
    bool completed;
    bool success;
    char message[DATA_SIZE];
} WriteStatus;

// Global write status tracking
static WriteStatus current_write = {0};
static pthread_mutex_t write_status_mutex = PTHREAD_MUTEX_INITIALIZER;

int processWriteResponse(Message *response, char *content, int write_content);

#endif