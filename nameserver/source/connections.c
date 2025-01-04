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
        connection_caller(client_socket, &initial_message, process_client_request);
    }
    else if (sender == 'S')
    {
        connection_caller(client_socket, &initial_message, process_server_request);
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

char *exchangeMessage(const char *ip, int port, Message *request)
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

void connection_caller(int client_socket, Message *initial_message, void (*process_request)(char *, size_t, char, struct sockaddr_in *, int, int))
{
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    if (getpeername(client_socket, (struct sockaddr *)&address, &addrlen) < 0)
    {
        perror("Failed to get peer name");
        log_system_event("Error", "Failed to get peer name");
        return;
    }

    // Initialize the RequestBuffer
    RequestBuffer *requestBuffer = init_request_buffer();
    if (!requestBuffer)
    {
        return;
    }

    Message message;
    memset(&message, 0, sizeof(message));
    memcpy(&message, initial_message, sizeof(Message));

    while (1)
    {
        message.data[2047] = 0;
        if (message.datasize > sizeof(message.data))
        {
            fprintf(stderr, "Invalid data size received from client %s:%d.\n",
                    inet_ntoa(address.sin_addr),
                    ntohs(address.sin_port));
            char toWrite[BUFSIZ];
            snprintf(toWrite, BUFSIZ, "Invalid data size received from client %s:%d.\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            log_system_event("Error", toWrite);
            break;
        }

        // If it's the first packet of a new request, initialize the buffer
        if (message.packetNo == 1)
        {
            reset_request_buffer(requestBuffer);
            requestBuffer->expectedTotal = message.totalPackets;
            requestBuffer->receivedPackets = 0;
        }
        else
        {
            // Ensure that we're in the middle of assembling a request
            if (requestBuffer->expectedTotal == 0)
            {
                fprintf(stderr, "Received packet %d without starting a new request from client %s:%d.\n",
                        message.packetNo,
                        inet_ntoa(address.sin_addr),
                        ntohs(address.sin_port));
                char toWrite[BUFSIZ];
                snprintf(toWrite, BUFSIZ, "Received packet %d without starting a new request from client %s:%d.\n", message.packetNo, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                log_system_event("Error", toWrite);
                break;
            }

            // Optional: Verify that the packetNo is within expected range
            if (message.packetNo < 1 || message.packetNo > requestBuffer->expectedTotal)
            {
                fprintf(stderr, "Received out-of-range packet number %d from client %s:%d.\n",
                        message.packetNo,
                        inet_ntoa(address.sin_addr),
                        ntohs(address.sin_port));
                char toWrite[BUFSIZ];
                snprintf(toWrite, BUFSIZ, "Received out-of-range packet number %d from client %s:%d.\n", message.packetNo, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                log_system_event("Error", toWrite);
                break;
            }
        }
        // Append the data from the current packet to the request buffer
        if (append_to_request_buffer(requestBuffer, message.data, message.datasize) < 0)
        {
            fprintf(stderr, "Failed to append data to request buffer for client %s:%d.\n",
                    inet_ntoa(address.sin_addr),
                    ntohs(address.sin_port));
            char toWrite[BUFSIZ];
            snprintf(toWrite, BUFSIZ, "Failed to append data to request buffer for client %s:%d.\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            log_system_event("Error", toWrite);
            break;
        }
        requestBuffer->receivedPackets++;

        // If all packets have been received, process the complete request
        if (requestBuffer->receivedPackets >= requestBuffer->expectedTotal)
        {

            // Process the complete request
            process_request(requestBuffer->data, requestBuffer->size, message.sender, &address, client_socket, 1);

            // Reset the buffer for the next request
            reset_request_buffer(requestBuffer);
        }

        // Read the next message
        ssize_t valread = recv(client_socket, &message, sizeof(Message), MSG_WAITALL);
        if (valread > 0)
        {
            continue; // Continue processing the next message
        }
        else if (valread == 0)
        {
            // Connection closed by the client
            printf("Client disconnected: IP %s, Port %d\n",
                   inet_ntoa(address.sin_addr),
                   ntohs(address.sin_port));
            char toWrite[BUFSIZ];
            snprintf(toWrite, BUFSIZ, "Client disconnected: IP %s, Port %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            log_system_event("Info", toWrite);
            break;
        }
        else
        {
            // Error occurred during recv
            perror("Receive failed");
            log_system_event("Error", "Receive failed");
            break;
        }
    }

    // Clean up
    reset_request_buffer(requestBuffer);
    free(requestBuffer);
}
