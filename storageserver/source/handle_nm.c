#include "../header.h"

RequestBuffer *init_request_buffer()
{
    RequestBuffer *buffer = malloc(sizeof(RequestBuffer));
    if (!buffer)
    {
        perror("Failed to allocate memory for RequestBuffer");
        return NULL;
    }
    memset(buffer, 0, sizeof(RequestBuffer));
    buffer->data = NULL;
    buffer->size = 0;
    buffer->capacity = 0;
    buffer->expectedTotal = 0;
    buffer->receivedPackets = 0;
    return buffer;
}

/**
 * Appends data to the RequestBuffer, expanding it as necessary.
 *
 * @param buffer Pointer to the RequestBuffer.
 * @param data Pointer to the data to append.
 * @param datasize Size of the data to append.
 * @return 0 on success, -1 on failure.
 */
int append_to_request_buffer(RequestBuffer *buffer, char *data, size_t datasize)
{
    if (!buffer || !data || datasize == 0)
    {
        return -1;
    }

    size_t new_size = buffer->size + datasize;

    if (new_size > buffer->capacity)
    {
        // Double the capacity or increase by the needed amount, whichever is larger
        size_t new_capacity = buffer->capacity == 0 ? datasize : buffer->capacity;
        while (new_capacity < new_size)
        {
            new_capacity *= 2;
        }

        char *new_data = NULL;
        if (buffer->data == NULL)
        {
            new_data = malloc(new_capacity);
            memset(new_data, 0, new_capacity);
        }
        else
        {

            new_data = realloc(buffer->data, new_capacity);
        }
        if (!new_data)
        {
            perror("Failed to reallocate buffer");
            return -1;
        }

        buffer->data = new_data;
        buffer->capacity = new_capacity;
    }

    memcpy(buffer->data + buffer->size, data, datasize + 1);
    buffer->size = new_size;
    buffer->receivedPackets++;

    return 0;
}

/**
 * Resets the RequestBuffer to start assembling a new request.
 *
 * @param buffer Pointer to the RequestBuffer.
 */
void reset_request_buffer(RequestBuffer *buffer)
{
    free(buffer->data);
    buffer->data = NULL;
    buffer->size = 0;
    buffer->capacity = 0;
    buffer->expectedTotal = 0;
    buffer->receivedPackets = 0;
}

void handle_nm(int nm_socket, Message *initial_message)
{
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    if (getpeername(nm_socket, (struct sockaddr *)&address, &addrlen) < 0)
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
            Message response;
            response.sender = 'S';
            response.packetNo = 1;
            response.totalPackets = 1;
            snprintf(response.data, sizeof(response.data), "I am alive\n");
            response.datasize = strlen(response.data);
            send(nm_socket, &response, sizeof(Message), 0);

            // Reset the buffer for the next request
            reset_request_buffer(requestBuffer);
        }

        ssize_t valread = recv(nm_socket, &message, sizeof(Message), MSG_WAITALL);
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
    reset_request_buffer(requestBuffer);
    free(requestBuffer);
}
