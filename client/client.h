#ifndef CLIENT_H
#define CLIENT_H

#define DATA_SIZE 2048

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

// Message structure for communication between client, NM, and SS
typedef struct Message
{
    char sender;          // Sender: 'C' (Client), 'N' (Naming Server), 'S' (Storage Server)
    int packetNo;         // Current packet number
    int totalPackets;     // Total number of packets
    int datasize;         // Data size in bytes
    char data[DATA_SIZE]; // Actual data
} Message;

typedef struct Msg_music
{
    char sender;          // Sender: 'C' (Client), 'N' (Naming Server), 'S' (Storage Server)
    int packetNo;         // Current packet number
    int totalPackets;     // Total number of packets
    int datasize;         // Data size in bytes
    char data[DATA_SIZE]; // Actual data
} Msg_music;

#endif // CLIENT_H