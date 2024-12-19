
#include "../header.h"
#define BUFFER_SIZE 2048

// Add this prototype for the new function
void stream_file(char *path, int client_socket);

char *inputParser(char *input, int socket)
{
    int inputsize = strlen(input);
    char newinput[inputsize];

    strcpy(newinput, input);

    Message msg_send;
    memset(&msg_send, 0, sizeof(Message));
    msg_send.sender = 'S';
    msg_send.packetNo = 1;
    msg_send.totalPackets = 1;

    char *savePtr;
    char *command = strtok_r(newinput, " ", &savePtr);
    if (command)
    {
        char *path = strtok_r(NULL, " ", &savePtr);
        if (path)
        {
            if (!strcmp(command, "READ"))
            {
                read_file(path, socket);
            }
            else if (!strcmp(command, "WRITE"))
            {
                // WRITE LOCATION MODE CONTENT
                int mode = atoi(strtok_r(NULL, " ", &savePtr));
                char *content = strtok_r(NULL, "\0", &savePtr);
                write_file(path, mode, socket, content);
            }
            else if (!strcmp(command, "STREAM"))
            {
                stream_file(path, socket); // Call the new function for STREAM
            }
            else if (!strcmp(command, "SNP"))
            {
                get_data(path, socket);
                // printf("Get - Command: %s, path: %s\n", command, path);
            }
            else
            {
                snprintf(msg_send.data, BUFFER_SIZE, "Incorrect command provided. Please try again.\n");
                msg_send.datasize = strlen(msg_send.data);
                printf("%s", msg_send.data);
                send(socket, &msg_send, sizeof(Message), 0);
                memset(&msg_send, 0, sizeof(Message));
            }
        }
        else
        {
            snprintf(msg_send.data, BUFFER_SIZE, "Please provide the path to the file\n" RESET);
            msg_send.datasize = strlen(msg_send.data);
            send(socket, &msg_send, sizeof(Message), 0);
            memset(&msg_send, 0, sizeof(Message));
        }
    }
    else
    {
        snprintf(msg_send.data, BUFFER_SIZE, "No command provided.\n" RESET);
        msg_send.datasize = strlen(msg_send.data);
        send(socket, &msg_send, sizeof(Message), 0);
        memset(&msg_send, 0, sizeof(Message));
    }
    return NULL;
}

// int main(){
//     char input[4096];
//     fgets(input, sizeof(input), stdin);
//     input[strcspn(input, "\n")] = '\0';
//     inputParser(input);
// }

char *nmParser(char *input, int socket)
{
    int inputsize = strlen(input);
    char newinput[inputsize];

    strcpy(newinput, input);

    Message msg_send;
    memset(&msg_send, 0, sizeof(Message));
    msg_send.sender = 'S';
    msg_send.packetNo = 1;
    msg_send.totalPackets = 1;

    char *savePtr;
    char *command = strtok_r(newinput, " ", &savePtr);
    if (command)
    {
        char *path = strtok_r(NULL, " ", &savePtr);
        if (path)
        {
            if (!strcmp(command, "CREATE"))
            {
                create_file(path, socket);
            }
            else if (!strcmp(command, "DELETE"))
            {
                delete_file(path, socket);
            }
            else
            {
                snprintf(msg_send.data, BUFFER_SIZE, "Incorrect command provided. Please try again.\n");
                msg_send.datasize = strlen(msg_send.data);
                printf("%s", msg_send.data);
                send(socket, &msg_send, sizeof(Message), 0);
                memset(&msg_send, 0, sizeof(Message));
            }
        }
        else
        {
            snprintf(msg_send.data, BUFFER_SIZE, "Please provide the path to the file\n" RESET);
            msg_send.datasize = strlen(msg_send.data);
            send(socket, &msg_send, sizeof(Message), 0);
            memset(&msg_send, 0, sizeof(Message));
        }
    }
    else
    {
        snprintf(msg_send.data, BUFFER_SIZE, "No command provided.\n" RESET);
        msg_send.datasize = strlen(msg_send.data);
        send(socket, &msg_send, sizeof(Message), 0);
        memset(&msg_send, 0, sizeof(Message));
    }
    return NULL;
}