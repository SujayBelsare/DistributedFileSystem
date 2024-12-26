#ifndef MESSAGE_H
#define MESSAGE_H

#define DATA_SIZE 2048

typedef struct Message
{
    char sender;          // Sender: 'C' (Client), 'N' (Naming Server), 'S' (Storage Server)
    int packetNo;         // Current packet number
    int totalPackets;     // Total number of packets
    int datasize;         // Data size in bytes
    char data[DATA_SIZE]; // Actual data
} Message;

typedef struct
{
    char ip[16];
    int port;
} ServerInfo;

#endif