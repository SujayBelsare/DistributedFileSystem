#include "../header/main.h"
void delete_file(Node *child, int client_socket, int flag)
{

    pthread_mutex_lock(&shared_data_mutex);
    Connection *main = fileArray[child->metadata->number].main;
    Connection *backup1 = fileArray[child->metadata->number].backup1;
    Connection *backup2 = fileArray[child->metadata->number].backup2;
    char *main_ip = NULL;

    int main_port = 0;
    Message mainreq;
    Message backreq1;
    Message backreq2;

    if (main == NULL && backup1 == NULL && backup2 == NULL)
    {
        printf("No server defined.\n");
        log_system_event("Error", "No server defined");
        Message response;
        response.sender = 'N';
        response.packetNo = 1;
        response.totalPackets = 1;
        snprintf(response.data, sizeof(response.data), "ERROR: NO SERVER DEFINED");
        response.datasize = strlen(response.data);
        send(client_socket, &response, sizeof(Message), 0);
        return;
    }
    else if (main != NULL)
    {
        main->filecount++;
        fileArray[child->metadata->number].main = main;

        main_ip = inet_ntoa(main->address.sin_addr);
        main_port = ntohs(main->address.sin_port);
        mainreq.sender = 'N';
        mainreq.packetNo = 1;
        mainreq.totalPackets = 1;
        mainreq.datasize = snprintf(mainreq.data, DATA_SIZE, "DELETE %d", child->metadata->number);
    }
    if (backup1 != NULL && backup2 != NULL)
    {
        backup1->filecount++;
        backup2->filecount++;

        fileArray[child->metadata->number].backup1 = backup1;
        fileArray[child->metadata->number].backup2 = backup2;

        backreq1.sender = 'N';
        backreq1.packetNo = 1;
        backreq1.totalPackets = 1;
        backreq1.datasize = snprintf(backreq1.data, DATA_SIZE, "DELETE %d", child->metadata->number);

        backreq2.sender = 'N';
        backreq2.packetNo = 1;
        backreq2.totalPackets = 1;
        backreq2.datasize = snprintf(backreq2.data, DATA_SIZE, "DELETE %d", child->metadata->number);
    }
    pthread_mutex_unlock(&shared_data_mutex);

    exchangeMessage(main_ip, main_port, &mainreq);

    if (backup1 != NULL && backup2 != NULL)
    {
        exchangeMessage(inet_ntoa(backup1->address.sin_addr), ntohs(backup1->address.sin_port), &backreq1);
        exchangeMessage(inet_ntoa(backup2->address.sin_addr), ntohs(backup2->address.sin_port), &backreq2);
    }

    if (flag == 2) // message is not to be sent back to client
    {
        return;
    }
    Message response;
    response.sender = 'N';
    response.packetNo = 1;
    response.totalPackets = 1;
    snprintf(response.data, sizeof(response.data), "FILE DELETED SUCCESSFULLY.");
    log_system_event("Info", "File Deleted successfully");
    response.datasize = strlen(response.data);
    send(client_socket, &response, sizeof(Message), 0);

    return;
}
