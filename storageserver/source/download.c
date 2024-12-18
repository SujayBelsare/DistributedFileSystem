#include "../header.h"

// order of received data is: DOWNLOAD <filename> <MODE> <SOCKET> DATA <data>
void *download_file(char *file, int mode, int socket, char* content)
{
    Message msg_send;
    memset(&msg_send, 0, sizeof(Message));
    msg_send.sender = 'S';
    msg_send.packetNo = 1;
    msg_send.totalPackets = 1;

    FILE* fd = fopen(file, mode == 0 ? "wb" : "ab");
    if(fd < 0){
        snprintf(msg_send.data, BUFFER_SIZE, RED"Error creating file\n"RESET);
        msg_send.datasize = strlen(msg_send.data);
        perror(msg_send.data);
        send(socket, &msg_send, sizeof(Message), 0);
        memset(&msg_send, 0, sizeof(Message));
        return NULL;
    }

    strncpy(msg_send.data, content, strlen(content));
    msg_send.datasize = strlen(msg_send.data);
    msg_send.packetNo = 1;
    msg_send.totalPackets = 1;

    if(fwrite(msg_send.data, 1, msg_send.datasize, fd) != msg_send.datasize) {
        perror(RED"fwrite"RESET);
        snprintf(msg_send.data, BUFFER_SIZE, RED"Error: fwrite failed\n"RESET);
        msg_send.datasize = strlen(msg_send.data);
        send(socket, &msg_send, sizeof(Message), 0);
        memset(&msg_send, 0, sizeof(Message));
        fclose(fd);
        return NULL;
    }

    fclose(fd);
    snprintf(msg_send.data, BUFFER_SIZE, GREEN"File downloaded successfully\n"RESET);
    msg_send.datasize = strlen(msg_send.data);
    printf("%s", msg_send.data);
    send(socket, &msg_send, sizeof(Message), 0);
    memset(&msg_send, 0, sizeof(Message));
    return NULL;
}