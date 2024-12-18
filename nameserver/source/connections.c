#include "../header/main.h"

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
