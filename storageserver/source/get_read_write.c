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
        close(socket);
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
        close(socket);
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
        close(socket);
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
    memset(buffer, 0, sizeof(buffer));

    close(socket);
    return NULL;
}

void *write_file(char *path, int socket)
{
    Message msg;
    memset(&msg, 0, sizeof(Message));
    msg.sender = 'S';
    msg.packetNo = 1;
    msg.totalPackets = 1;

    path[strcspn(path, "\n")] = '\0';
    struct stat statbuf;

    printf("Path: %s\n", path);
    if (stat(path, &statbuf) == -1)
    {
        snprintf(msg.data, BUFFER_SIZE, RED "Error: stat failed\n" RESET);
        msg.datasize = strlen(msg.data);
        msg.packetNo = 1;
        msg.totalPackets = 1;
        perror(RED "stat" RESET);
        send(socket, &msg, sizeof(Message), 0);
        memset(&msg, 0, sizeof(Message));
        close(socket);
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
        close(socket);
        return NULL;
    }

    FILE *file = fopen(path, "a");
    if (!file)
    {
        snprintf(msg.data, BUFFER_SIZE, RED "Error: fopen failed\n" RESET);
        msg.datasize = strlen(msg.data);
        msg.packetNo = 1;
        msg.totalPackets = 1;
        perror(RED "fopen" RESET);
        send(socket, &msg, sizeof(Message), 0);
        memset(&msg, 0, sizeof(Message));
        close(socket);
        return NULL;
    }

    snprintf(msg.data, BUFFER_SIZE, GREEN "INPUT THE CONTENT:\n" RESET);
    msg.datasize = strlen(msg.data);
    msg.packetNo = 1;
    msg.totalPackets = 1;
    send(socket, &msg, sizeof(Message), 0);

    char buffer[2048];
    recv(socket, buffer, sizeof(buffer), 0);
    while (strcmp(buffer, "STOP\n") != 0)
    {
        // write buffer to file
        fprintf(file, "%s", buffer);
        printf("Data: %s\n", buffer);
        fflush(file);
        memset(buffer, 0, sizeof(buffer));
        if (recv(socket, buffer, sizeof(buffer), 0) < 0)
        {
            printf("Recieve Failed.\n");
        }
    }

    printf("\n\nThe Data is done.\n");
    close(socket);

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