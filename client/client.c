#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include "client.h"
#include <sys/time.h>
#include <pthread.h>

#include "clientheader.h"

#define ASYNC_THRESHOLD 1024 * 1024   // 1MB threshold for async writes
#define WRITE_STATUS_CHECK_INTERVAL 2 // seconds

typedef enum
{
    CMD_STREAM,
    CMD_CREATE,
    CMD_READ,
    CMD_DELETE,
    CMD_COPY,
    CMD_WRITE_STATUS,
    CMD_DETAILS,
    CMD_UNKNOWN,
    CMD_LIST
} CommandType;

typedef struct
{
    char ip[16];
    int port;
} ServerInfo;

typedef struct
{
    int request_id;
    bool completed;
    bool success;
    char message[DATA_SIZE];
} WriteStatus;

// Global write status tracking
static WriteStatus current_write = {0};
static pthread_mutex_t write_status_mutex = PTHREAD_MUTEX_INITIALIZER;

// Error handling function
static void handle_error(const char *message, bool fatal)
{
    fprintf(stderr, "Error: %s - %s\n", message, strerror(errno));
    if (fatal)
    {
        exit(EXIT_FAILURE);
    }
}

// Create and connect socket
int connect_to_server(const char *ip, int port)
{
    int sock;
    struct sockaddr_in server_addr;
    int retries = 10;
    while (retries-- > 0)
    {
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            handle_error("Socket creation failed", false);
            continue;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
        {
            close(sock);
            handle_error("Invalid address", true);
        }

        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0)
        {
            return sock;
        }

        close(sock);
    }

    handle_error("Connection failed after retries", true);
    return -1;
}

// Generic function to send/receive messages
bool exchange_messages(const char *ip, int port, Message *request, Message *response)
{
    int sock = connect_to_server(ip, port);
    bool success = false;
    if (send(sock, request, sizeof(Message), MSG_NOSIGNAL) < 0)
    {
        handle_error("Send failed", false);
        goto cleanup;
    }

    if (recv(sock, response, sizeof(Message), 0) < 0)
    {
        handle_error("Receive failed", false);
        goto cleanup;
    }
    if (strncmp(response->data, "DATA", 4) == 0)
    {
        printf("READ from file:%s\n", response->data + 4);
    }
    success = true;

cleanup:
    close(sock);
    return success;
}

static void stream_music(const char *ss_ip, int ss_port, const char *path)
{
    int sock = connect_to_server(ss_ip, ss_port);

    // Send the stream request
    Message message = {
        .sender = 'C',
        .packetNo = 1,
        .totalPackets = 1,
        .datasize = strlen(path)};
    strncpy(message.data, path, DATA_SIZE - 1);
    message.data[DATA_SIZE - 1] = '\0';

    if (send(sock, &message, sizeof(Message), MSG_NOSIGNAL) < 0)
    {
        handle_error("Failed to send stream request", false);
        close(sock);
        return;
    }

    FILE *player = popen("mpv --no-terminal -", "w");
    if (!player)
    {
        handle_error("Failed to start music player", false);
        close(sock);
        return;
    }

    // Receive total packet count
    Message response;
    ssize_t bytes_received = recv(sock, &response, sizeof(Message), 0);
    if (bytes_received <= 0 || response.totalPackets <= 0)
    {
        printf("%s\n", response.data);
        close(sock);
        pclose(player);
        return;
    }

    int total_packets = response.totalPackets;
    printf("Total packets to receive: %d\n", total_packets);

    // Receive and play each packet
    int packets_received = 0;
    while (packets_received < total_packets)
    {
        bytes_received = recv(sock, &response, sizeof(Message), 0);
        if (bytes_received <= 0)
        {
            perror("Error receiving stream data");
            break;
        }

        packets_received++;
        if (fwrite(response.data, 1, response.datasize, player) != response.datasize)
        {
            handle_error("Failed to write to player", false);
            break;
        }
    }

    if (packets_received == total_packets)
    {
        printf("All packets received. Stream complete.\n");
    }
    else
    {
        printf("Stream incomplete. Received %d/%d packets.\n", packets_received, total_packets);
    }

    pclose(player);
    close(sock);
}

// Function to check write status
static void *check_write_status(void *arg)
{
    ServerInfo *nm_info = (ServerInfo *)arg;
    Message request = {
        .sender = 'C',
        .packetNo = 1,
        .totalPackets = 1,
        .datasize = strlen("WRITE_STATUS")};
    strncpy(request.data, "WRITE_STATUS", DATA_SIZE - 1);

    while (1)
    {
        sleep(WRITE_STATUS_CHECK_INTERVAL);

        pthread_mutex_lock(&write_status_mutex);
        if (current_write.completed)
        {
            pthread_mutex_unlock(&write_status_mutex);
            break;
        }
        pthread_mutex_unlock(&write_status_mutex);

        Message response;
        if (exchange_messages(nm_info->ip, nm_info->port, &request, &response))
        {
            pthread_mutex_lock(&write_status_mutex);
            if (strncmp(response.data, "WRITE_FAILED", 11) == 0)
            {
                current_write.success = false;
                current_write.completed = true;
                strncpy(current_write.message, response.data, DATA_SIZE);
                printf("Write operation failed: %s\n", response.data);
            }
            else if (strncmp(response.data, "WRITE_COMPLETE", 13) == 0)
            {
                current_write.success = true;
                current_write.completed = true;
                strncpy(current_write.message, "Write operation completed successfully", DATA_SIZE);
                printf("Write operation completed successfully\n");
            }
            pthread_mutex_unlock(&write_status_mutex);
        }
    }

    free(nm_info);
    return NULL;
}

static void handle_write_request(const char *command, const ServerInfo *storage_server, const ServerInfo *naming_server)
{
    bool is_sync = strstr(command, "--SYNC") != NULL;
    size_t data_size = strlen(command); // In real implementation, this would be the actual data size

    Message request = {
        .sender = 'C',
        .packetNo = 1,
        .totalPackets = 1,
        .datasize = strlen(command)};

    // Add sync flag to request if specified
    if (is_sync)
    {
        strncpy(request.data, command, DATA_SIZE - 7);
        strcat(request.data, " --SYNC");
    }
    else
    {
        strncpy(request.data, command, DATA_SIZE - 1);
    }

    Message response;
    if (!exchange_messages(storage_server->ip, storage_server->port, &request, &response))
    {
        handle_error("Failed to communicate with Storage Server", true);
        return;
    }

    // Handle synchronous write
    if (is_sync)
    {
        printf("Synchronous write response: %s\n", response.data);
        return;
    }

    // Handle asynchronous write
    if (strncmp(response.data, "WRITE_ACCEPTED", 13) == 0)
    {
        printf("Write request accepted asynchronously\n");

        // Start monitoring thread for write status
        pthread_t status_thread;
        ServerInfo *nm_info = malloc(sizeof(ServerInfo));
        memcpy(nm_info, naming_server, sizeof(ServerInfo));

        if (pthread_create(&status_thread, NULL, check_write_status, nm_info) != 0)
        {
            handle_error("Failed to create status monitoring thread", false);
            free(nm_info);
            return;
        }
        pthread_detach(status_thread);
    }
    else
    {
        printf("Write request failed: %s\n", response.data);
    }
}

static CommandType parse_command(const char *command)
{
    static const struct
    {
        const char *cmd;
        CommandType type;
    } commands[] = {
        {"STREAM", CMD_STREAM},
        {"CREATE", CMD_CREATE},
        {"READ", CMD_READ},
        {"DELETE", CMD_DELETE},
        {"COPY", CMD_COPY},
        {"WRITE", CMD_WRITE_STATUS},
        {"DETAILS", CMD_DETAILS},
        {"LIST", CMD_LIST},

    }; // get file metadata

    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
    {
        if (strncmp(command, commands[i].cmd, strlen(commands[i].cmd)) == 0)
        {
            return commands[i].type;
        }
    }
    return CMD_UNKNOWN;
}

static bool parse_server_response(const char *response, ServerInfo *server)
{
    return (sscanf(response, "%s %d", server->ip, &server->port) == 2);
}

int main(void)
{
    const char nm_ip[] = "0.0.0.0";
    const int nm_port = 34000;
    ServerInfo naming_server = {.ip = "0.0.0.0", .port = 34000};
    char command[DATA_SIZE * 512];
    ServerInfo storage_server;

    // Initialize write status mutex
    pthread_mutex_init(&write_status_mutex, NULL);

    while (1)
    {
        char command[DATA_SIZE];
        printf("Enter command (or type EXIT to quit): ");
        memset(command, 0, DATA_SIZE);
        if (!fgets(command, DATA_SIZE, stdin))
        {
            handle_error("Failed to read command", true);
        }
        command[strcspn(command, "\n")] = '\0';

        // Check if user wants to exit
        if (strcmp(command, "EXIT") == 0)
        {
            printf("Exiting client.\n");
            break;
        }

        // Validate command type
        int valid_command = (strncmp(command, "CREATE", 6) == 0 ||
                             strncmp(command, "READ", 4) == 0 ||
                             strncmp(command, "STREAM", 6) == 0 ||
                             strncmp(command, "WRITE", 5) == 0 ||
                             strncmp(command, "DELETE", 6) == 0 ||
                             strncmp(command, "LIST", 4) == 0 ||
                             strncmp(command, "COPY", 4) == 0 ||
                             strncmp(command, "DETAILS", 7) == 0);

        if (!valid_command)
        {
            printf("ERROR: INVALID COMMAND\n");
            continue; // Redirect to the `while` loop for re-input
        }

        Message request = {
            .sender = 'C',
            .packetNo = 1,
            .totalPackets = 1,
            .datasize = strlen(command)};
        strncpy(request.data, command, DATA_SIZE - 1);
        request.data[DATA_SIZE - 1] = '\0';

        Message response;
        if (!exchange_messages(nm_ip, nm_port, &request, &response))
        {
            handle_error("Failed to communicate with Naming Server", true);
        }

        // Check for specific error codes in the response
        if (strncmp(response.data, "1001:", 5) == 0 || // Directory inside file
            strncmp(response.data, "1002:", 5) == 0 || // Invalid path
            strncmp(response.data, "1004:", 5) == 0 || // Resource already exists
            strncmp(response.data, "ERROR:", 6) == 0)  // Generic error
        {
            printf("Server Error: %s\n", response.data);
            continue; // Redirect to the `while` loop for re-input
        }

        printf("NM Response: %s\n", response.data);

        CommandType cmd_type = parse_command(command);
        switch (cmd_type)
        {
        case CMD_STREAM:
            if (!parse_server_response(response.data, &storage_server))
            {
                printf("Incorrect input, please try again.\n");
                continue;
            }
            stream_music(storage_server.ip, storage_server.port, command);
            break;

        case CMD_CREATE:
            processCreateResponse(&response);
            break;
        case CMD_READ:
            processReadResponse(&response);
            break;
        case CMD_LIST:
            processListResponse(&response);
            break;
        case CMD_WRITE_STATUS:
            printf("Command: %s\n", command);
            char temp1[50]; // Allocate memory for temp1
            char temp2[50]; // Allocate memory for temp2
            char temp3[50]; // Allocate memory for temp3
            char temp4[50]; // Allocate memory for temp4
            char content[1024 * 1024];
            int write_kind;
            // the current format is WRITE FILE <file_path> <file_name> <write_kind> <content>
            sscanf(command, "%s %s %s %s %d %[^\n]s", temp1, temp2, temp3, temp4, &write_kind, content);
            // printf("content:%s\n", content);
            size_t content_len = strlen(content);
            size_t total_chunks = (content_len + 2000 - 1) / 2000;
            size_t bytes_sent = 0;

            char content_to_be_sent[2048];
            strncpy(content_to_be_sent, content + bytes_sent, 2000);
            processWriteResponse(&response, content_to_be_sent, write_kind);
            int chunks = 1;
            bytes_sent += strlen(content_to_be_sent);
            while (chunks < total_chunks)
            {
                strncpy(content_to_be_sent, content + bytes_sent, 2000);
                processWriteResponse(&response, content_to_be_sent, 1);
                bytes_sent += strlen(content_to_be_sent);
            }
            break;
            // processReadResponse(&response);
        case CMD_DELETE:
            processDeleteResponse(&response);
            break;
        case CMD_COPY:
            // processCopyResponse(&response);
        case CMD_DETAILS:
            // syntax: DETAILS <file_path>
            processGetResponse(&response);
            break;
        default:
            handle_error("Unsupported command", true);
        }
    }
    pthread_mutex_destroy(&write_status_mutex);
    return EXIT_SUCCESS;
}
