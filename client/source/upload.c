#include "../headers/clientheader.h"

// order of uploading is: UPLOAD <filename> DATA <data>
void* upload_file(char* file, int socket){
    Message msg_send;
    memset(&msg_send, 0, sizeof(Message));
    msg_send.sender = 'C';
    msg_send.packetNo = 1;
    msg_send.totalPackets = 1;

    file[strcspn(file, "\n")] = '\0';
    struct stat statbuf;

    if(stat(file, &statbuf) == -1){
        snprintf(msg_send.data, BUFFER_SIZE, RED"Error: stat failed\n"RESET);
        msg_send.datasize = strlen(msg_send.data);
        perror(RED " stat" RESET);
        send(socket, &msg_send, sizeof(Message), 0);
        memset(&msg_send, 0, sizeof(Message));
        return NULL;
    }

    if(!S_ISREG(statbuf.st_mode)){
        snprintf(msg_send.data, BUFFER_SIZE, RED"The path provided does not exist\n"RESET);
        msg_send.datasize = strlen(msg_send.data);
        printf("%s", msg_send.data);
        send(socket, &msg_send, sizeof(Message), 0);
        memset(&msg_send, 0, sizeof(Message));
        return NULL;
    }

    FILE* fd = fopen(file, "rb");
    if(fd < 0){
        snprintf(msg_send.data, BUFFER_SIZE, RED"Error opening file\n"RESET);
        msg_send.datasize = strlen(msg_send.data);
        perror(RED"fopen"RESET);
        send(socket, &msg_send, sizeof(Message), 0);
        memset(&msg_send, 0, sizeof(Message));
        return NULL;
    }

    int file_size = statbuf.st_size;
    int totalPackets = (file_size + 2000 -1) / 2000;

    int packetNo = 1;
    while(fgets(msg_send.data, 2000, fd)){
        char data[BUFFER_SIZE] = {0};
        strcpy(data, "UPLOAD ");
        strcat(data, msg_send.data);
        strcpy(msg_send.data, data);
        msg_send.packetNo = packetNo++;
        msg_send.totalPackets = totalPackets;
        msg_send.datasize = strlen(msg_send.data);
        send(socket, &msg_send, sizeof(Message), 0);
        memset(&msg_send, 0, sizeof(Message));
    }
    fclose(fd);

    snprintf(msg_send.data, BUFFER_SIZE, GREEN"File uploaded successfully\n"RESET);
    msg_send.datasize = strlen(msg_send.data);
    msg_send.packetNo = totalPackets;
    msg_send.totalPackets = totalPackets;
    printf("%s", msg_send.data);
    send(socket, &msg_send, sizeof(Message), 0);
    memset(&msg_send, 0, sizeof(Message));
    return NULL;
}