#include "../headers/clientheader.h"

int processWriteResponse(Message *response, char *content, int write_kind)
{
    int fileNo;
    char swtch;
    char mainIP[16];
    int mainPort;

    char backupIP1[16];
    int backupPort1;

    char backupIP2[16];
    int backupPort2;

    char *data[2048];

    sscanf(response->data, "%c", &swtch);
    if (swtch == '1')
    {
        sscanf(response->data, "1%s %d %d", mainIP, &mainPort, &fileNo);
    }
    else
    {
        sscanf(response->data, "2%s %d %d %s %d %d %s %d %d",
               mainIP, &mainPort, &fileNo,
               backupIP1, &backupPort1, &fileNo,
               backupIP2, &backupPort2, &fileNo);
    }
    Message request1;
    Message request2;
    Message request3;

    request1.datasize = snprintf(request1.data, DATA_SIZE, "WRITE %d %d %s", fileNo, write_kind, content);
    request2.datasize = snprintf(request2.data, DATA_SIZE, "WRITE %d %d %s", fileNo, write_kind, content);
    request3.datasize = snprintf(request3.data, DATA_SIZE, "WRITE %d %d %s", fileNo, write_kind, content);

    request1.packetNo = 1;
    request1.totalPackets = 1;
    request1.sender = 'C';

    request2.packetNo = 1;
    request2.totalPackets = 1;
    request2.sender = 'C';

    request3.packetNo = 1;
    request3.totalPackets = 1;
    request3.sender = 'C';

    Message response1;
    Message response2;
    Message response3;

    exchange_messages(mainIP, mainPort, &request1, &response1);
    if (swtch == '1')
    {
        return 0;
    }
    exchange_messages(backupIP1, backupPort1, &request2, &response2);
    exchange_messages(backupIP2, backupPort2, &request3, &response3);
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
