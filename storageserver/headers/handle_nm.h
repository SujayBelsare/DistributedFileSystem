#ifndef _HANDLE_NM__
#define _HANDLE_NM__

#include "../header.h"

void handle_nm(int nm_socket, Message* initial_message);

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