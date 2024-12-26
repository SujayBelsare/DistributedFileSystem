#include "../headers/clientheader.h"

int processCopyResponse(Message *initial_respose, char *nmip, int nmport)
{
    char *bigdata = malloc(1000000);
    memset(bigdata, 0, 1000000);
    int numPackets = (1000000 / 2048) + 1;
    // recieve numPackets - 1 from here, response is the first packet, keep on appending the data to bigdata
    int sock = connect_to_server(nmip, nmport);
    strcat(bigdata, initial_respose->data);
    for (int i = 0; i < numPackets - 1; i++)
    {
        Message response;
        ssize_t bytes_received = recv(sock, &response, sizeof(Message), 0);
        if (bytes_received <= 0)
        {
            perror("Error receiving stream data");
            break;
        }
        strcat(bigdata, response.data);
    }

    char *saveptr;
    char *line = strtok_r(bigdata, "\n", &saveptr);

    while (line != NULL)
    {
        // for each path here,
        // generate a new path by appending the line to the destination path
        // send the new path to the name server (nm)
        // do this: create a file for new path,
    }
}

int processListResponse(Message *response)
{
    // just print response data if it's not null
    if (response->data != NULL)
    {
        printf("%s\n", response->data);
    }
}