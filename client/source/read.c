#include "../headers/clientheader.h"

int processReadResponse(Message *response)
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

    request1.datasize = snprintf(request1.data, DATA_SIZE, "READ %d", fileNo);
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

    char buffer[DATA_SIZE];
    recv(sock, buffer, sizeof(buffer), 0);
    while (strcmp(buffer, "STOP") != 0)
    {
        printf("%s", buffer);
        if (recv(sock, buffer, sizeof(buffer), 0) < 0)
        {
            printf("Recieve Failed.\n");
        }
    }

    printf("\n\nThe Data is done.\n");
    close(sock);
    return 0;
}