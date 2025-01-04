#ifndef MESSAGE_H
#define MESSAGE_H
#include "../header.h"

typedef struct Message
{
    char sender;      // who is sending. S : Storage Server. N : Name Server. C : Client
    int packetNo;     // the current packet number of the data
    int totalPackets; // the total number of packets the sender is expected to send.
    int datasize;     // the number of bytes of data the sender is sending (in the data field)
    char data[2048];  // actual data
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

void reset_request_buffer(RequestBuffer *buffer);

int append_to_request_buffer(RequestBuffer *buffer, char *data, size_t datasize);

#endif