#include "../headers/clientheader.h"

#include "../headers/clientheader.h"

int processWriteResponse(Message *response)
{
    int fileNo;

    char mainIP[16];
    int mainPort;

    // char backupIP1[16];
    // int backupPort1;

    // char backupIP2[16];
    // int backupPort2;

    Message request1;
    Message request2;
    Message request3;

    if (response->data[0] == '3')
    {
        printf("Response from the server: %s\n", response->data + 1);
        return 0;
    }

    sscanf(response->data, "%s %d %d", mainIP, &mainPort, &fileNo);

    request1.datasize = snprintf(request1.data, DATA_SIZE, "WRITE %d", fileNo);
    // request2.datasize = snprintf(request2.data, DATA_SIZE, "READ %d", fileNo);
    // request3.datasize = snprintf(request3.data, DATA_SIZE, "READ %d", fileNo);

    request1.packetNo = 1;
    request1.totalPackets = 1;
    request1.sender = 'C';

    // request2.packetNo = 1;
    // request2.totalPackets = 1;
    // request2.sender = 'C';

    // request3.packetNo = 1;
    // request3.totalPackets = 1;
    // request3.sender = 'C';

    Message response1;
    // Message response2;
    // Message response3;

    int sock = connect_to_server(mainIP, mainPort);
    bool success = false;
    if (send(sock, &request1, sizeof(Message), MSG_NOSIGNAL) < 0)
    {
        // printf("Send failed", false);
        printf("Send Failed.\n");
    }

    if (recv(sock, response, sizeof(Message), 0) < 0)
    {
        printf("Recieve Failed.\n");
    }
    printf("%s\n", response->data);

    size_t bytes_read;
    char buffer[2048] = {0};
    while (fgets(buffer, sizeof(buffer), stdin))
    {
        printf("Data: %s\n", buffer);
        if (send(sock, buffer, sizeof(buffer), 0) == -1)
        {
            perror("send failed");
            break;
        }
        if (buffer[strlen(buffer) - 1] != '\n')
        {
            continue;
        }
        if (strcmp(buffer, "STOP\n") == 0)
        {
            break;
        }
        memset(buffer, 0, sizeof(buffer));
    }

    printf("\n\nThe is written successfully.\n");

    close(sock);
    return 0;
}

void *check_write_status(void *arg)
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
        int temp = exchange_messages(nm_info->ip, nm_info->port, &request, &response);
        if (temp)
        {
            close(temp);
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
