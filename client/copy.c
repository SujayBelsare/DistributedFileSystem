#include "copy.h"

// Error handling function
static void handle_error(const char *message, bool fatal)
{
    fprintf(stderr, "Error: %s - %s\n", message, strerror(errno));
    if (fatal)
    {
        exit(EXIT_FAILURE);
    }
}

// int connect_to_server(const char *ip, int port)
// {
//     int sock;
//     struct sockaddr_in server_addr;
//     int retries = 10;
//     while (retries-- > 0)
//     {
//         if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
//         {
//             handle_error("Socket creation failed", false);
//             continue;
//         }

//         server_addr.sin_family = AF_INET;
//         server_addr.sin_port = htons(port);

//         if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
//         {
//             close(sock);
//             handle_error("Invalid address", true);
//         }

//         if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0)
//         {
//             return sock;
//         }

//         close(sock);
//     }

//     handle_error("Connection failed after retries", true);
//     return -1;
// }

// int processCopyResponse(Message *initial_respose, char *nmip, int nmport)
// {
//     char *bigdata = malloc(1000000);
//     memset(bigdata, 0, 1000000);
//     int numPackets = (1000000 / 2048) + 1;
//     // recieve numPackets - 1 from here, response is the first packet, keep on appending the data to bigdata
//     int sock = connect_to_server(nmip, nmport);
//     strcat(bigdata, initial_respose->data);
//     for (int i = 0; i < numPackets - 1; i++)
//     {
//         Message response;
//         ssize_t bytes_received = recv(sock, &response, sizeof(Message), 0);
//         if (bytes_received <= 0)
//         {
//             perror("Error receiving stream data");
//             break;
//         }
//         strcat(bigdata, response.data);
//     }

//     char *saveptr;
//     char *line = strtok_r(bigdata, "\n", &saveptr);

//     while(line!=NULL){
//         // for each path here,
//         // generate a new path by appending the line to the destination path
//         // send the new path to the name server (nm)
//         // do this: create a file for new path,

//     }
// }

int processListResponse(Message *response)
{
    // just print response data if it's not null
    if (response->data != NULL)
    {
        printf("%s\n", response->data);
    }
}