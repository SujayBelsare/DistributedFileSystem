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
