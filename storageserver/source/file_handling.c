#include "../header.h"

void *read_file(char *path, int socket)
{
    Message msg;
    memset(&msg, 0, sizeof(Message));
    msg.sender = 'S';
    msg.packetNo = 1;
    msg.totalPackets = 1;

    path[strcspn(path, "\n")] = '\0';
    struct stat statbuf;

    if (stat(path, &statbuf) == -1)
    {
        snprintf(msg.data, BUFFER_SIZE, RED "Error: stat failed\n" RESET);
        msg.datasize = strlen(msg.data);
        msg.packetNo = 1;
        msg.totalPackets = 1;
        perror(RED "stat" RESET);
        send(socket, &msg, sizeof(Message), 0);
        memset(&msg, 0, sizeof(Message));
        return NULL;
    }

    if (!S_ISREG(statbuf.st_mode))
    {
        snprintf(msg.data, BUFFER_SIZE, RED "The path provided does not exist\n" RESET);
        msg.datasize = strlen(msg.data);
        msg.packetNo = 1;
        msg.totalPackets = 1;
        printf("%s", msg.data);
        send(socket, &msg, sizeof(Message), 0);
        memset(&msg, 0, sizeof(Message));
        return NULL;
    }

    FILE *file = fopen(path, "r");
    if (!file)
    {
        snprintf(msg.data, BUFFER_SIZE, RED "Error: fopen failed\n" RESET);
        msg.datasize = strlen(msg.data);
        msg.packetNo = 1;
        msg.totalPackets = 1;
        perror(RED "fopen" RESET);
        send(socket, &msg, sizeof(Message), 0);
        memset(&msg, 0, sizeof(Message));
        return NULL;
    }

    int file_size = statbuf.st_size;
    int totalPackets = (file_size + 2000 - 1) / 2000;

    int packetNo = 1;
    size_t bytes_read;
    while ((bytes_read = fread(msg.data, 1, 2000, file)) > 0)
    {
        char data[BUFFER_SIZE] = {0};
        strcpy(data, "DATA ");
        strcat(data, msg.data);
        strcpy(msg.data, data);
        msg.packetNo = packetNo++;
        msg.totalPackets = totalPackets;
        msg.datasize = strlen(msg.data);
        send(socket, &msg, sizeof(Message), 0);
        memset(&msg, 0, sizeof(Message));
    }
    fclose(file);

    snprintf(msg.data, BUFFER_SIZE, GREEN "\nFile read successfully\n" RESET);
    msg.datasize = strlen(msg.data);
    msg.packetNo = totalPackets;
    msg.totalPackets = totalPackets;
    printf("%s", msg.data);
    send(socket, &msg, sizeof(Message), 0);
    memset(&msg, 0, sizeof(Message));

    return NULL;
}

void *write_file(char *path, int mode, int socket, char *content)
{
    Message msg_send;
    memset(&msg_send, 0, sizeof(Message));
    msg_send.sender = 'S';
    msg_send.packetNo = 1;
    msg_send.totalPackets = 1;

    FILE *file = fopen(path, mode == 0 ? "wb" : "ab");
    if (!file)
    {
        perror(RED "fopen" RESET);
        snprintf(msg_send.data, BUFFER_SIZE, RED "Error: fopen failed\n" RESET);
        msg_send.datasize = strlen(msg_send.data);
        msg_send.packetNo = 1;
        msg_send.totalPackets = 1;
        send(socket, &msg_send, sizeof(Message), 0);
        memset(&msg_send, 0, sizeof(Message));
        return NULL;
    }

    int content_length = strlen(content);
    // int total_packets = (content_length + 256 - 1) / 256;
    // int packetNo = 1;

    for(int i = 0; i < content_length; i += 256)
    {
        int chunk_size = content_length - i < 256 ? content_length - i : 256;
        strncpy(msg_send.data, content + i, 256);
        if (fwrite(msg_send.data, 1, msg_send.datasize, file) != msg_send.datasize)
        {
            perror(RED "fwrite" RESET);
            snprintf(msg_send.data, BUFFER_SIZE, RED "Error: fwrite failed\n" RESET);
            msg_send.datasize = strlen(msg_send.data);
            send(socket, &msg_send, sizeof(Message), 0);
            memset(&msg_send, 0, sizeof(Message));
            fclose(file);
            return NULL;
        }
        memset(&msg_send, 0, sizeof(Message));
    }

    snprintf(msg_send.data, BUFFER_SIZE, GREEN "String written to file successfully\n" RESET);
    msg_send.datasize = strlen(msg_send.data);
    msg_send.packetNo = 1;
    msg_send.totalPackets = 1;
    printf("%s", msg_send.data);
    send(socket, &msg_send, sizeof(Message), 0);
    fclose(file);

    return NULL;
}

#define DATA_SIZE 2048 // Must match client's DATA_SIZE

// to be modified

void *stream_file(char *path, int socket, int client_socket)
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

    // Assuming file is in: storage server/source/testing/jungle.mp3
    snprintf(path, BUFFER_SIZE, "%s/source/testing/jungle.mp3", base_path);
    printf("Path to stream: %s\n", path);

    // Validate the file
    struct stat statbuf;
    if (stat(path, &statbuf) == -1)
    {
        perror(RED "stat failed" RESET);
        snprintf(buffer, BUFFER_SIZE, "Error: File not found or inaccessible: %s", path);
        send(client_socket, buffer, strlen(buffer), MSG_NOSIGNAL);
        close(client_socket);
        return NULL;
    }

    if (!S_ISREG(statbuf.st_mode))
    {
        snprintf(buffer, BUFFER_SIZE, "Error: The path provided is not a valid file");
        send(client_socket, buffer, strlen(buffer), MSG_NOSIGNAL);
        close(client_socket);
        return NULL;
    }

    FILE *file = fopen(path, "rb");
    if (!file)
    {
        perror(RED "fopen failed" RESET);
        snprintf(buffer, BUFFER_SIZE, "Error: Couldn't open file");
        send(client_socket, buffer, strlen(buffer), MSG_NOSIGNAL);
        close(client_socket);
        return NULL;
    }

    // Calculate total packets needed
    long file_size = statbuf.st_size;
    int total_packets = (file_size + DATA_SIZE - 1) / DATA_SIZE;
    printf("File size: %ld bytes, Total packets: %d\n", file_size, total_packets);

    // Initialize message structure
    Message msg = {
        .sender = 'S',
        .packetNo = 0,
        .totalPackets = total_packets,
        .datasize = 0};

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


void permissions(mode_t mode, int socket, int *packetNo, int totalPackets)
{
    Message msg;
    memset(&msg, 0, sizeof(Message));
    msg.sender = 'S';
    msg.packetNo = *packetNo;
    msg.totalPackets = totalPackets;

    snprintf(msg.data, BUFFER_SIZE, "Permissions: %c%c%c%c%c%c%c%c%c\n",
             (mode & S_IRUSR) ? 'r' : '-',
             (mode & S_IWUSR) ? 'w' : '-',
             (mode & S_IXUSR) ? 'x' : '-',
             (mode & S_IRGRP) ? 'r' : '-',
             (mode & S_IWGRP) ? 'w' : '-',
             (mode & S_IXGRP) ? 'x' : '-',
             (mode & S_IROTH) ? 'r' : '-',
             (mode & S_IWOTH) ? 'w' : '-',
             (mode & S_IXOTH) ? 'x' : '-');
    msg.datasize = strlen(msg.data);
    printf("%s", msg.data);
    send(socket, &msg, sizeof(Message), 0);
    (*packetNo)++;
}

void *get_data(char *path, int socket)
{
    Message msg;
    memset(&msg, 0, sizeof(Message));
    msg.sender = 'S';
    msg.packetNo = 1;
    msg.totalPackets = 1;

    path[strcspn(path, "\n")] = '\0';
    struct stat file_stats;

    if (stat(path, &file_stats) == -1)
    {
        perror(RED "stat" RESET);
        snprintf(msg.data, BUFFER_SIZE, RED "Error: stat failed\n" RESET);
        msg.datasize = strlen(msg.data);
        send(socket, &msg, sizeof(Message), 0);
        return NULL;
    }

    int packetNo = 1;
    int totalPackets = 8;

    snprintf(msg.data, BUFFER_SIZE, BLUE "File path: " RESET "%s\n", path);
    msg.datasize = strlen(msg.data);
    msg.packetNo = packetNo++;
    msg.totalPackets = totalPackets;
    printf("%s", msg.data);
    send(socket, &msg, sizeof(Message), 0);

    snprintf(msg.data, BUFFER_SIZE, BLUE "File size: " RESET "%lld bytes\n", (long long)file_stats.st_size);
    msg.datasize = strlen(msg.data);
    msg.packetNo = packetNo++;
    msg.totalPackets = totalPackets;
    printf("%s", msg.data);
    send(socket, &msg, sizeof(Message), 0);

    snprintf(msg.data, BUFFER_SIZE, BLUE "File type: " RESET);
    msg.datasize = strlen(msg.data);
    msg.packetNo = packetNo++;
    msg.totalPackets = totalPackets;
    printf("%s", msg.data);
    send(socket, &msg, sizeof(Message), 0);

    if (S_ISREG(file_stats.st_mode))
    {
        snprintf(msg.data, BUFFER_SIZE, "Regular file\n");
    }
    else if (S_ISDIR(file_stats.st_mode))
    {
        snprintf(msg.data, BUFFER_SIZE, "Directory\n");
    }
    else if (S_ISCHR(file_stats.st_mode))
    {
        snprintf(msg.data, BUFFER_SIZE, "Character device\n");
    }
    else if (S_ISBLK(file_stats.st_mode))
    {
        snprintf(msg.data, BUFFER_SIZE, "Block device\n");
    }
    else if (S_ISFIFO(file_stats.st_mode))
    {
        snprintf(msg.data, BUFFER_SIZE, "FIFO/pipe\n");
    }
    else if (S_ISLNK(file_stats.st_mode))
    {
        snprintf(msg.data, BUFFER_SIZE, "Symbolic link\n");
    }
    else if (S_ISSOCK(file_stats.st_mode))
    {
        snprintf(msg.data, BUFFER_SIZE, "Socket\n");
    }
    else
    {
        snprintf(msg.data, BUFFER_SIZE, "Unknown\n");
    }
    msg.datasize = strlen(msg.data);
    msg.packetNo = packetNo++;
    msg.totalPackets = totalPackets;
    printf("%s", msg.data);
    send(socket, &msg, sizeof(Message), 0);

    permissions(file_stats.st_mode, socket, &packetNo, totalPackets);

    struct passwd* pw = getpwuid(file_stats.st_uid);
    struct group* gr = getgrgid(file_stats.st_gid);

    snprintf(msg.data, BUFFER_SIZE, MAGENTA "Owner:" RESET " %s\n", pw ? pw->pw_name : "Unknown");
    msg.datasize = strlen(msg.data);
    msg.packetNo = packetNo++;
    msg.totalPackets = totalPackets;
    printf("%s", msg.data);
    send(socket, &msg, sizeof(Message), 0);

    snprintf(msg.data, BUFFER_SIZE, MAGENTA "Group: " RESET " %s\n", gr ? gr->gr_name : "Unknown");
    msg.datasize = strlen(msg.data);
    msg.packetNo = packetNo++;
    msg.totalPackets = totalPackets;
    printf("%s", msg.data);
    send(socket, &msg, sizeof(Message), 0);

    // Last modification time
    snprintf(msg.data, BUFFER_SIZE, WHITE "Last modified: " RESET "%s", ctime(&file_stats.st_mtime));
    msg.datasize = strlen(msg.data);
    msg.packetNo = packetNo++;
    msg.totalPackets = totalPackets;
    printf("%s", msg.data);
    send(socket, &msg, sizeof(Message), 0);

    return NULL;
}

// int main(){
//     char path[256];
//     fgets(path, sizeof(path), stdin);
//     read_file(path);
// }