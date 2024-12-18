#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // close()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>  // inet_ntoa()
#include <pthread.h>

#define SO_REUSEPORT 15

typedef struct Message
{
    char sender;      // who is sending. S : Storage Server. N : Name Server. C : Client
    int packetNo;     // the current packet number of the data
    int totalPackets; // the total number of packets the sender is expected to send.
    int datasize;     // the number of bytes of data the sender is sending (in the data field)
    char data[2048];  // actual data
} Message;
