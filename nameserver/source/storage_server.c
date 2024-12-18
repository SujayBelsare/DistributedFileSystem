#include "../header/main.h"

extern Node *root;
extern int server_num;
extern MinHeap *mainHeap;
extern Server *fileArray;
extern Connection *serverArray;

void handle_server(int client_socket, Message *initial_message)
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
    memset(&message, 0, sizeof(Message));
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
        printf("DEBUG-1\n");
        // If all packets have been received, process the complete request
        if (requestBuffer->receivedPackets >= requestBuffer->expectedTotal)
        {
            printf("DEBUG-2\n");
            // Process the complete request
            process_server_request(requestBuffer->data, requestBuffer->size, message.sender, &address, client_socket);
            printf("DEBUG-3\n");
            // Reset the buffer for the next request
            reset_request_buffer(requestBuffer);
        }

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
    reset_request_buffer(requestBuffer);
    free(requestBuffer);
}

// create a lock
pthread_mutex_t serverArrayMutex = PTHREAD_MUTEX_INITIALIZER;
void process_server_request(char *data, size_t size, char sender, struct sockaddr_in *address, int client_socket)
{
    char *token;
    char *ip_val = __strtok_r(data, " \n,", &token);
    char *port_val = __strtok_r(NULL, " \n,", &token);
    char *files_val = __strtok_r(NULL, " \n,", &token);
    printf("DEBUG-40\n");
    struct sockaddr_in* server_addr = malloc(sizeof(struct sockaddr_in));
    memset(server_addr, 0, sizeof(struct sockaddr_in));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(atoi(port_val));
    printf("DEBUG-41\n");
    if (inet_pton(AF_INET, ip_val, &server_addr->sin_addr) <= 0) {
        perror("Invalid Addr / Addr not supported");
        log_system_event("Error", "Invalid Addr / Addr not supported");
        return;
    }
    Connection *new = malloc(sizeof(Connection));
    memset(new, 0, sizeof(sizeof(Connection)));
    new->address = *server_addr;
    new->address.sin_addr = address->sin_addr;
    new->socket = client_socket;
    printf("DEBUG-30\n");
    new->filecount = atoi(files_val);
    pthread_mutex_lock(&serverArrayMutex);
    printf("DEBUG-31\n");
    new->server_id = server_num;
    serverArray[server_num] = *new;
    server_num++;
    printf("%s %d\n", inet_ntoa(address->sin_addr), ntohs(new->address.sin_port));
    char toWrite[BUFSIZ];
    snprintf(toWrite, BUFSIZ, "New server connected: IP %s, Port %d\n", inet_ntoa(address->sin_addr), ntohs(new->address.sin_port));
    log_system_event("Info", toWrite);
    insert(mainHeap, new);
    
    pthread_mutex_unlock(&serverArrayMutex);
    printf("DEBUG-32\n");
    Message response;
    memset(&response, 0, sizeof(Message));
    response.sender = 'N';
    response.packetNo = 1;
    response.totalPackets = 1;
    snprintf(response.data, sizeof(response.data), "OK");
    printf("DEBUG-33\n");
    response.datasize = strlen(response.data);
    send(client_socket, &response, sizeof(Message), 0);
    return;
}
