#include "../header/main.h"
#include <stdbool.h>

extern FILE *log_file;

void *connection_handler(void *arg)
{
    int client_socket = *((int *)arg);
    free(arg);

    // Read initial message
    Message initial_message;
    memset(&initial_message, 0, sizeof(Message));
    ssize_t bytes_read = recv(client_socket, &initial_message, sizeof(Message), 0);
    initial_message.data[initial_message.datasize] = 0;
    if (bytes_read <= 0)
    {
        log_system_event("Error", "Failed to read initial message");
        close_logging(log_file);
        perror("Failed to read initial message");
        close(client_socket);
        return NULL;
    }

    char sender = initial_message.sender;
    if (sender == 'C')
    {
        handle_client(client_socket, &initial_message);
    }
    else if (sender == 'S')
    {
        handle_server(client_socket, &initial_message);
    }
    else
    {
        fprintf(stderr, "Unknown sender type: %c\n", sender);
        log_system_event("Error", "Unknown sender type");
    }

    close(client_socket);
    return NULL;
}

int connect_to_server(const char *ip, int port)
{
    int sock;
    struct sockaddr_in server_addr;
    int retries = 10;
    while (retries-- > 0)
    {
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            continue;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
        {
            close(sock);
        }

        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0)
        {
            return sock;
        }

        close(sock);
    }

    return -1;
}

char* exchangeMessage(const char *ip, int port, Message *request)
{
    int sock = connect_to_server(ip, port);
    int success = 0;
    printf("SENDING: %s\n", request->data);

    if (send(sock, request, sizeof(Message), MSG_NOSIGNAL) < 0)
    {
        close(sock);
        return NULL;
    }
    success = 1;
    printf("DEBUG-1\n");

    Message response;
    memset(&response, 0, sizeof(Message));
    ssize_t bytes_read = recv(sock, &response, sizeof(Message), 0);
    response.data[response.datasize] = 0;
    if (bytes_read <= 0)
    {
        close(sock);
        return NULL;
    }
    close(sock);

    return NULL;
}