#include "../headers/clientheader.h"

int processCopyResponse(Message *response, int client_socket)
{
    // just print response data if it's not null
    while (response->data != NULL && strcmp(response->data, "STOP") != 0)
    {
        printf("here rn\n");
        printf("%s\n", response->data);
        recv(client_socket, response, sizeof(Message), 0);
    }

    return 0;
}