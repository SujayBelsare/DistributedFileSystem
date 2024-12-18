#ifndef IDENTIFIERS_H
#define IDENTIFIERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // close()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // inet_ntoa()
#include <pthread.h>

typedef struct
{
    int socket;
    struct sockaddr_in address;
    int filecount;
    int server_id;
} Connection;

typedef struct
{
    Connection *main;
    Connection *backup1;
    Connection *backup2;
} Server;

typedef struct
{
    Connection **array; // Array of pointers to Connection
    int size;
    int capacity;
} MinHeap;

MinHeap *createMinHeap();
void insert(MinHeap *heap, Connection *connection);
void minHeapify(MinHeap *heap, int i);
Connection *extractMin(MinHeap *heap);

#endif