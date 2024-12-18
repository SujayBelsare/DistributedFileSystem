#include "delete.h"

static void handle_error(const char *message, bool fatal)
{
    fprintf(stderr, "Error: %s - %s\n", message, strerror(errno));
    if (fatal)
    {
        exit(EXIT_FAILURE);
    }
}

bool get_messages_delete(const char *ip, int port, Message *response)
{
    int sock = connect_to_server(ip, port);
    bool success = false;
    printf("DEBUG-DEL1\n");

    Message sending;
    strcpy(sending.data, "hello, its me!");
    sending.datasize = strlen(sending.data);
    sending.packetNo = 1;
    sending.sender = 'C';
    sending.totalPackets = 1;

    if (send(sock, &sending, sizeof(Message), MSG_NOSIGNAL) < 0)
    {
        handle_error("Send failed", false);
        goto cleanup;
    }

    if (recv(sock, response, sizeof(Message), 0) < 0)
    {
        handle_error("Receive failed", false);
        goto cleanup;
    }
    success = true;
    printf("DEBUG-DEL2: %s\n", response->data);
cleanup:
    close(sock);
    return success;
}

bool exchange_messages_delete(const char *ip, int port, Message *request, Message *response, const char* ip_nm, int port_nm)
{
    int sock = connect_to_server(ip, port);
    int sock_nm = connect_to_server(ip_nm, port_nm);
    Message sending;
    strcpy(sending.data, "hello, its me!");
    sending.datasize = strlen(sending.data);
    sending.packetNo = 1;
    sending.sender = 'C';
    sending.totalPackets = 1;
    printf("SENDING NM: %s\n", sending.data);
    if (send(sock_nm, &sending, sizeof(Message), MSG_NOSIGNAL) < 0)
    {
        handle_error("Send failed", false);
    }
    printf("KJNDFJKNDF\n");
    if (recv(sock_nm, response, sizeof(Message), 0) < 0)
    {
        handle_error("Receive failed", false);
    }
    printf("RECEIVED NM: %s\n", response->data);
    bool success = false;
    printf("SENDING SS: %s\n", request->data);
    if (send(sock, request, sizeof(Message), MSG_NOSIGNAL) < 0)
    {
        handle_error("Send failed", false);
        goto cleanup;
    }

    if (recv(sock, response, sizeof(Message), 0) < 0)
    {
        handle_error("Receive failed", false);
        goto cleanup;
    }
    printf("RECEIVED SS: %s\n", response->data);
    if (strncmp(response->data, "DATA", 4) == 0)
    {
        printf("READ from file:%s\n", response->data + 4);
    }
    success = true;

cleanup:
    close(sock);
    return success;
}

int processDeleteResponse(Message *response, const char *nmIP, int nmPort)
{
    int fileNo;
    char swtch;

    char mainIP[16];
    int mainPort;

    char backupIP1[16];
    int backupPort1;

    char backupIP2[16];
    int backupPort2;

    int containsStop = 0;
    while (1)
    {
        printf("Response from the server: %s\n", response->data);
        if (strstr(response->data, "STOP"))
        {
            printf("DEBUG-DEL\n");
            containsStop = 1;
            // int sock = connect_to_server(nmIP, nmPort);
            // Message inform;
            // inform.totalPackets = 1;
            // inform.packetNo = 1;
            // inform.sender = 'C';
            // strcpy(inform.data, "DELETE DONE");
            // inform.datasize = strlen(inform.data);
            // send(sock, &inform, sizeof(Message), MSG_NOSIGNAL);
            return 0;
        }

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

        request1.datasize = snprintf(request1.data, DATA_SIZE, "DELETE %d", fileNo);
        request2.datasize = snprintf(request2.data, DATA_SIZE, "DELETE %d", fileNo);
        request3.datasize = snprintf(request3.data, DATA_SIZE, "DELETE %d", fileNo);

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

        exchange_messages_delete(mainIP, mainPort, &request1, &response1, nmIP, nmPort);
        if (swtch == '1')
        {
            // get_messages_delete(nmIP, nmPort, response);
            continue;
        }
        exchange_messages(backupIP1, backupPort1, &request2, &response2);
        exchange_messages(backupIP2, backupPort2, &request3, &response3);

        get_messages_delete(nmIP, nmPort, response);
    }
}