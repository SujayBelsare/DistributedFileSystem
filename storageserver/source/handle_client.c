// Keep in mind thread locks while reading/writing to/from files.

#include "../header.h"

void process_client_request(char *data, size_t size, char sender, struct sockaddr_in *address, int client_socket);

void handle_client(int client_socket, Message *initial_message)
{
    
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    if (getpeername(client_socket, (struct sockaddr *)&address, &addrlen) < 0)
    {
        perror("Failed to get peer name");
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
        // Process the current message
        // Convert integer fields from network byte order to host byte order
        // message.packetNo = atoi(message.packetNo);
        // message.totalPackets = atoi(message.totalPackets);
        // message.datasize = atoi(message.datasize);

        // Validate datasize to prevent buffer overflow
        message.data[2047] = 0;
        // printf("%c\n %d\n %d\n %d\n %s\n", message.sender, message.packetNo, message.totalPackets, message.datasize, message.data);
        // printf("%d\n", message.datasize);
        // printf("%s\n", message.data);
        // printf("%ld\n", sizeof(message.data));
        if (message.datasize > sizeof(message.data))
        {
            fprintf(stderr, "Invalid data size received from client %s:%d.\n",
                    inet_ntoa(address.sin_addr),
                    ntohs(address.sin_port));
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
                break;
            }

            // Optional: Verify that the packetNo is within expected range
            if (message.packetNo < 1 || message.packetNo > requestBuffer->expectedTotal)
            {
                fprintf(stderr, "Received out-of-range packet number %d from client %s:%d.\n",
                        message.packetNo,
                        inet_ntoa(address.sin_addr),
                        ntohs(address.sin_port));
                break;
            }
        }

        // Append the data from the current packet to the request buffer
        if (append_to_request_buffer(requestBuffer, message.data, message.datasize) < 0)
        {
            fprintf(stderr, "Failed to append data to request buffer for client %s:%d.\n",
                    inet_ntoa(address.sin_addr),
                    ntohs(address.sin_port));
            break;
        }

        requestBuffer->receivedPackets++;

        // If all packets have been received, process the complete request
        if (requestBuffer->receivedPackets >= requestBuffer->expectedTotal)
        {
            // Process the complete request
            
            process_client_request(requestBuffer->data, requestBuffer->size, message.sender, &address, client_socket);
            // Reset the buffer for the next request
            reset_request_buffer(requestBuffer);
            return;
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
            break;
        }
        else
        {
            // Error occurred during recv
            perror("Receive failed");
            break;
        }
    }

    // Clean up
    reset_request_buffer(requestBuffer);
    free(requestBuffer);
}

void process_client_request(char *data, size_t size, char sender, struct sockaddr_in *address, int client_socket)
{
    // Print incoming request information
    printf("Received request from client %s:%d\n",
           inet_ntoa(address->sin_addr),
           ntohs(address->sin_port));
    printf("Request data: %s\n", data);

    // Initialize response structure
    Message response;
    memset(&response, 0, sizeof(Message));
    response.sender = 'S';
    response.packetNo = 1;
    response.totalPackets = 1;

    // Validate input data
    if (data == NULL || size == 0) {
        snprintf(response.data, sizeof(response.data), "ERR_104: Invalid Command");
        response.datasize = strlen(response.data);
        send(client_socket, &response, sizeof(Message), MSG_NOSIGNAL);
        return;
    }

    // Process command using inputParser
    inputParser(data, client_socket);

    // Handle inputParser errors
    // if (parse_result == -1) { // Example error case from inputParser
    //     snprintf(response.data, sizeof(response.data), "ERR_100: File Not Found");
    //     response.datasize = strlen(response.data);
    //     send(client_socket, &response, sizeof(Message), MSG_NOSIGNAL);
    //     return;
    // } else if (parse_result == -2) {
    //     snprintf(response.data, sizeof(response.data), "ERR_102: File Locked");
    //     response.datasize = strlen(response.data);
    //     send(client_socket, &response, sizeof(Message), MSG_NOSIGNAL);
    //     return;
    // }

    // If the command is processed successfully
    snprintf(response.data, sizeof(response.data), "OK");
    response.datasize = strlen(response.data);

    
    // send(client_socket, &response, sizeof(Message), 0);
}
