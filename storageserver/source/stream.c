#include "../header.h"

void *stream_file(char *path, int client_socket)
{

    char buffer[BUFFER_SIZE];
    int bytes_sent;

    // Construct the correct path dynamically assuming the server executable is outside "source"
    char base_path[1024];
    if (getcwd(base_path, sizeof(base_path)) == NULL)
    {
        perror("getcwd() error");
        snprintf(buffer, BUFFER_SIZE, "Error: Unable to get current working directory");
        send(client_socket, buffer, strlen(buffer), MSG_NOSIGNAL);
        close(client_socket);
        return NULL;
    }

    // Assuming file is in: storage server/source/music/jungle.mp3
    char temp[512];
    strcpy(temp, path);
    snprintf(path, BUFFER_SIZE, "%s/source/%s", base_path, temp);
    printf("Path to stream: %s\n", path);
    Message msg = {
        .sender = 'S',
        .packetNo = 1,
        .totalPackets = 0,
        .datasize = 0};

    // Validate the file
    struct stat statbuf;
    if (stat(path, &statbuf) == -1)
    {
        perror(RED "stat failed:" RESET);
        snprintf(msg.data, BUFFER_SIZE, "Error: File not found or inaccessible.");
        msg.datasize = strlen(msg.data);
        send(client_socket, &msg, sizeof(Message), MSG_NOSIGNAL);
        close(client_socket);
        return NULL;
    }

    if (!S_ISREG(statbuf.st_mode))
    {
        snprintf(msg.data, BUFFER_SIZE, "Error: The path provided is not a valid file.");
        msg.datasize = strlen(msg.data);
        send(client_socket, &msg, sizeof(Message), MSG_NOSIGNAL);

        close(client_socket);
        return NULL;
    }

    FILE *file = fopen(path, "rb");
    if (!file)
    {
        perror(RED "fopen failed" RESET);
        snprintf(msg.data, BUFFER_SIZE, "Error: Couldnt open file.");
        msg.datasize = strlen(msg.data);
        send(client_socket, &msg, sizeof(Message), MSG_NOSIGNAL);

        close(client_socket);
        return NULL;
    }

    // Calculate total packets needed
    long file_size = statbuf.st_size;
    int total_packets = (file_size + DATA_SIZE - 1) / DATA_SIZE;
    printf("File size: %ld bytes, Total packets: %d\n", file_size, total_packets);

    // Initialize message structure
    msg.totalPackets = total_packets;
    // Stream file in chunks
    size_t bytes_read;
    while ((bytes_read = fread(msg.data, 1, DATA_SIZE, file)) > 0)
    {
        msg.packetNo++;
        msg.datasize = bytes_read;

        printf("Sending packet %d/%d, Size: %zu bytes\n", msg.packetNo, msg.totalPackets, bytes_read);

        if (send(client_socket, &msg, sizeof(Message), MSG_NOSIGNAL) < 0)
        {
            perror(RED "send failed" RESET);
            fclose(file);
            close(client_socket);
            return NULL;
        }

        memset(msg.data, 0, DATA_SIZE); // Clear buffer
    }

    if (ferror(file))
    {
        perror(RED "Error reading file" RESET);
        fclose(file);
        close(client_socket);
        return NULL;
    }

    // Send end-of-stream message
    msg.datasize = 4;
    strncpy(msg.data, "STOP", 4);
    if (send(client_socket, &msg, sizeof(Message), MSG_NOSIGNAL) < 0)
    {
        perror(RED "send STOP failed" RESET);
    }

    fclose(file);
    printf(YELLOW "File streamed successfully\n" RESET);
    return NULL;
}
