// Handles all comms with the nameserver
#include "../header.h"

// extern void handle_client(int nm_socket, Message* initial_message);
// extern void handle_nm(int nm_socket, Message* initial_message);

void *nm_sm_handler(void *arg)
{
    int nm_socket = *((int *)arg);
    free(arg);

    // Read initial message
    Message initial_message;
    memset(&initial_message, 0, sizeof(Message));
    ssize_t bytes_read = recv(nm_socket, &initial_message, sizeof(Message), 0);
    initial_message.data[initial_message.datasize] = 0;
    if (bytes_read <= 0)
    {
        perror("Failed to read initial message");
        close(nm_socket);
        return NULL;
    }

    char sender = initial_message.sender;
    if (sender == 'C')
    {
        printf("DEBUG-9\n");
        handle_client(nm_socket, &initial_message);
        printf("CLIENT SE AAYA HEIN\n");
    }
    else if (sender == 'N')
    {
        handle_nm(nm_socket, &initial_message);
    }
    else
    {
        fprintf(stderr, "Unknown sender type: %c\n", sender);
    }

    close(nm_socket);
    return NULL;
}

int main()
{
    int storageServerFD, new_socket;
    char nameserver_ip[16];
    int nameserver_port;
    int max_files;

    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    // Create socket
    if ((storageServerFD = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(storageServerFD, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("Setsockopt failed");
        close(storageServerFD);
        exit(EXIT_FAILURE);
    }

    // Bind socket to dynamic port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(35000); // DEBUG: Hardcoded port

    if (bind(storageServerFD, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        close(storageServerFD);
        exit(EXIT_FAILURE);
    }

    // Retrieve assigned port
    if (getsockname(storageServerFD, (struct sockaddr *)&address, &addrlen) == -1)
    {
        perror("getsockname failed");
        close(storageServerFD);
        exit(EXIT_FAILURE);
    }

    printf("Enter Naming Server IP: ");
    scanf("%s", nameserver_ip);
    printf("Enter Naming Server Port: ");
    scanf("%d", &nameserver_port);
    printf("Enter Max Number of Files: ");
    scanf("%d", &max_files);

    // send a message to the nameserver
    Message firstMessage;
    memset(&firstMessage, 0, sizeof(Message));
    firstMessage.sender = 'S';
    firstMessage.packetNo = 1;
    firstMessage.totalPackets = 1;

    snprintf(firstMessage.data, sizeof(firstMessage.data), "%s %d %d", inet_ntoa(address.sin_addr), ntohs(address.sin_port), max_files);
    firstMessage.datasize = strlen(firstMessage.data);

    // send message to the nameserver
    int nm_socket;
    struct sockaddr_in nm_address;
    if ((nm_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    nm_address.sin_family = AF_INET;
    nm_address.sin_port = htons(nameserver_port);
    if (inet_pton(AF_INET, nameserver_ip, &nm_address.sin_addr) <= 0)
    {
        perror("Invalid address / Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(nm_socket, (struct sockaddr *)&nm_address, sizeof(nm_address)) < 0)
    {
        perror("Connection to NM failed");
        exit(EXIT_FAILURE);
    }

    send(nm_socket, &firstMessage, sizeof(Message), 0);

    // recieve the response from the nameserver
    Message response;
    memset(&response, 0, sizeof(Message));
    ssize_t bytes_read = recv(nm_socket, &response, sizeof(Message), 0);
    response.data[response.datasize] = 0;
    if (bytes_read <= 0)
    {
        perror("Failed to read response from nameserver");
        close(nm_socket);
        exit(EXIT_FAILURE);
    }
    close(nm_socket);

    printf("Server is listening on IP: %s, Port: %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

    // Start listening
    if (listen(storageServerFD, MAX_CLIENTS) < 0)
    {
        perror("Listen failed");
        close(storageServerFD);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for connections...\n");

    while (1)
    {
        // Accept new connection
        new_socket = accept(storageServerFD, (struct sockaddr *)&address, &addrlen);
        if (new_socket < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("New connection from IP: %s, Port: %d\n",
               inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        // Allocate socket descriptor for the thread
        int *pclient = malloc(sizeof(int));
        if (pclient == NULL)
        {
            perror("Malloc failed");
            close(new_socket);
            continue;
        }
        *pclient = new_socket;

        // Create a new thread for the client
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, nm_sm_handler, pclient) != 0)
        {
            perror("Could not create thread");
            free(pclient);
            close(new_socket);
            continue;
        }

        // Detach the thread so it cleans up after itself
        pthread_detach(thread_id);
    }

    close(storageServerFD);
    return 0;
}