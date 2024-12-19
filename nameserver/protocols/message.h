#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // close()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>  // inet_ntoa()
#include <pthread.h>

#define DATA_SIZE 2048


typedef struct Message
{
    char sender;      // who is sending. S : Storage Server. N : Name Server. C : Client
    int packetNo;     // the current packet number of the data
    int totalPackets; // the total number of packets the sender is expected to send.
    int datasize;     // the number of bytes of data the sender is sending (in the data field)
    char data[DATA_SIZE];  // actual data
} Message;

typedef struct RequestBuffer
{
    char *data;          // Accumulated data
    size_t size;         // Current size of the accumulated data
    size_t capacity;     // Current capacity of the buffer
    int expectedTotal;   // Total number of packets expected
    int receivedPackets; // Number of packets received so far
} RequestBuffer;

RequestBuffer *init_request_buffer();

int append_to_request_buffer(RequestBuffer *buffer, char *data, size_t datasize);

void reset_request_buffer(RequestBuffer *buffer);

#endif