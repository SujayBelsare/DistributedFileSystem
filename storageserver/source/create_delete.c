#include "../header.h"

void *create_file(char *name, int socket)
{
    Message msg;
    memset(&msg, 0, sizeof(Message));
    msg.sender = 'S';
    msg.packetNo = 1;
    msg.totalPackets = 1;

    if (access(name, F_OK) == 0)
    {
        snprintf(msg.data, BUFFER_SIZE, RED "File already exists\n" RESET);
        msg.datasize = strlen(msg.data);
        printf("%s", msg.data);
        send(socket, &msg, sizeof(Message), 0);
        memset(&msg, 0, sizeof(Message));
        return NULL;
    }

    int fd = open(name, O_CREAT | O_WRONLY, 0666);
    if(fd < 0){
        snprintf(msg.data, BUFFER_SIZE, RED"Error creating file\n"RESET);
        msg.datasize = strlen(msg.data);
        perror(msg.data);
        send(socket, &msg, sizeof(Message), 0);
        memset(&msg, 0, sizeof(Message));
        return NULL;
    }
    close(fd);
    snprintf(msg.data, BUFFER_SIZE, GREEN"File created successfully\n"RESET);
    msg.datasize = strlen(msg.data);
    printf("%s", msg.data);
    send(socket, &msg, sizeof(Message), 0);
    memset(&msg, 0, sizeof(Message));
    return NULL;
}

void* delete_file(char* name, int socket){
    Message msg;
    memset(&msg, 0, sizeof(Message));
    msg.sender = 'S';
    msg.packetNo = 1;
    msg.totalPackets = 1;

    if(access(name, F_OK) < 0){
        snprintf(msg.data, BUFFER_SIZE, RED "File does not exist\n" RESET);
        msg.datasize = strlen(msg.data);
        printf("%s", msg.data);
        send(socket, &msg, sizeof(Message), 0);
        memset(&msg, 0, sizeof(Message));
        return NULL;
    }

    if(unlink(name) < 0){
        snprintf(msg.data, BUFFER_SIZE, RED "Error in deleting file\n" RESET);
        msg.datasize = strlen(msg.data);
        printf("%s", msg.data);
        send(socket, &msg, sizeof(Message), 0);
        memset(&msg, 0, sizeof(Message));
        return NULL;
    }

    snprintf(msg.data, BUFFER_SIZE, GREEN"File deleted successfully\n"RESET);
    msg.datasize = strlen(msg.data);
    printf("%s", msg.data);
    send(socket, &msg, sizeof(Message), 0);
    memset(&msg, 0, sizeof(Message));
    return NULL;
}