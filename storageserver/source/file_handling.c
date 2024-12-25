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

    snprintf(msg.data, BUFFER_SIZE, GREEN "FILE START:\n" RESET);
    msg.datasize = strlen(msg.data);
    msg.packetNo = 1;
    msg.totalPackets = 1;
    send(socket, &msg, sizeof(Message), 0);

    size_t bytes_read;
    char buffer[2048] = {0};
    while ((bytes_read = fread(buffer, 1, 2047, file)) > 0)
    {
        buffer[bytes_read] = '\0';
        send(socket, buffer, sizeof(buffer), 0);
        memset(buffer, 0, sizeof(buffer));
    }
    fclose(file);

    snprintf(buffer, BUFFER_SIZE, "STOP");
    send(socket, buffer, sizeof(buffer), 0);
    memset(&msg, 0, sizeof(buffer));

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

    for (int i = 0; i < content_length; i += 256)
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

    char temp[BUFFER_SIZE];

    snprintf(msg.data, BUFFER_SIZE, BLUE "File path: " RESET "%s\n", path);

    snprintf(temp, BUFFER_SIZE, BLUE "File size: " RESET "%lld bytes\n", (long long)file_stats.st_size);
    strcat(msg.data, temp);

    snprintf(temp, BUFFER_SIZE, BLUE "File type: " RESET);
    strcat(msg.data, temp);

    if (S_ISREG(file_stats.st_mode))
    {
        snprintf(temp, BUFFER_SIZE, "Regular file\n");
    }
    else if (S_ISDIR(file_stats.st_mode))
    {
        snprintf(temp, BUFFER_SIZE, "Directory\n");
    }
    else if (S_ISCHR(file_stats.st_mode))
    {
        snprintf(temp, BUFFER_SIZE, "Character device\n");
    }
    else if (S_ISBLK(file_stats.st_mode))
    {
        snprintf(temp, BUFFER_SIZE, "Block device\n");
    }
    else if (S_ISFIFO(file_stats.st_mode))
    {
        snprintf(temp, BUFFER_SIZE, "FIFO/pipe\n");
    }
    else if (S_ISLNK(file_stats.st_mode))
    {
        snprintf(temp, BUFFER_SIZE, "Symbolic link\n");
    }
    else if (S_ISSOCK(file_stats.st_mode))
    {
        snprintf(temp, BUFFER_SIZE, "Socket\n");
    }
    else
    {
        snprintf(temp, BUFFER_SIZE, "Unknown\n");
    }
    strcat(msg.data, temp);

    snprintf(temp, BUFFER_SIZE, "Permissions: %c%c%c%c%c%c%c%c%c\n",
             (file_stats.st_mode & S_IRUSR) ? 'r' : '-',
             (file_stats.st_mode & S_IWUSR) ? 'w' : '-',
             (file_stats.st_mode & S_IXUSR) ? 'x' : '-',
             (file_stats.st_mode & S_IRGRP) ? 'r' : '-',
             (file_stats.st_mode & S_IWGRP) ? 'w' : '-',
             (file_stats.st_mode & S_IXGRP) ? 'x' : '-',
             (file_stats.st_mode & S_IROTH) ? 'r' : '-',
             (file_stats.st_mode & S_IWOTH) ? 'w' : '-',
             (file_stats.st_mode & S_IXOTH) ? 'x' : '-');
    strcat(msg.data, temp);

    struct passwd *pw = getpwuid(file_stats.st_uid);
    struct group *gr = getgrgid(file_stats.st_gid);

    snprintf(temp, BUFFER_SIZE, MAGENTA "Owner:" RESET " %s\n", pw ? pw->pw_name : "Unknown");
    strcat(msg.data, temp);

    snprintf(temp, BUFFER_SIZE, MAGENTA "Group: " RESET " %s\n", gr ? gr->gr_name : "Unknown");
    strcat(msg.data, temp);

    // Last modification time
    snprintf(temp, BUFFER_SIZE, WHITE "Last modified: " RESET "%s", ctime(&file_stats.st_mtime));
    strcat(msg.data, temp);

    send(socket, &msg, sizeof(Message), 0);

    return NULL;
}

// int main(){
//     char path[256];
//     fgets(path, sizeof(path), stdin);
//     read_file(path);
// }