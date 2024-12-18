#include "header/main.h"

#define MAX_SIZE 100000

Node *root;
MinHeap *mainHeap;
int curr_num = 0;
int server_num = 0;
Server *fileArray;
Connection *serverArray;

Node *LRU_CACHE[HASHMAP_SIZE];

pthread_mutex_t trie_mutex = PTHREAD_MUTEX_INITIALIZER;
int main()
{
    FILE* log_file;
    init_logging(log_file);
    serverArray = malloc(MAX_SIZE * sizeof(Connection));
    memset(serverArray, 0, sizeof(serverArray));
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {   
        log_system_event("Error", "Socket failed");
        perror("Socket failed");
        close_logging(log_file);
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("Setsockopt failed");
        close(server_fd);
        log_system_event("Error", "Setsockopt failed");
        close_logging(log_file);
        exit(EXIT_FAILURE);
    }

    // Bind socket to dynamic port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(34000);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        log_system_event("Error", "Bind failed");
        perror("Bind failed");
        close(server_fd);
        close_logging(log_file);
        exit(EXIT_FAILURE);
    }

    // Retrieve assigned port
    if (getsockname(server_fd, (struct sockaddr *)&address, &addrlen) == -1)
    {
        perror("getsockname failed");
        close(server_fd);
        log_system_event("Error", "getsockname failed");
        close_logging(log_file);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on IP: %s, Port: %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
    char towrite[BUFSIZ];
    snprintf(towrite, BUFSIZ, "Server is listening on IP: %s, Port: %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
    log_system_event("Info", towrite);

    // Start listening
    if (listen(server_fd, MAX_CLIENTS) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        log_system_event("Error", "Listen failed");
        close_logging(log_file);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for connections...\n");
    root = restoreTrie();

    if (root == NULL)
    {
        root = createNode("/", NULL);
        root->metadata->isFile = 0;
        root->metadata->isDeleted = 0;
    }
    pthread_t save_thread = startAutoSave(root);

    mainHeap = createMinHeap();
    fileArray = (Server *)malloc(MAX_SIZE * sizeof(Server));
    memset(fileArray, 0, sizeof(Server));
    LRU_SETUP();
    while (1)
    {
        // Accept new connection
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket < 0)
        {
            perror("Accept failed");
            log_system_event("Error", "Accept failed");
            close_logging(log_file);
            continue;
        }

        printf("New connection from IP: %s, Port: %d\n",
               inet_ntoa(address.sin_addr), ntohs(address.sin_port));
        char towrite2[BUFSIZ];
        snprintf(towrite2, BUFSIZ, "New connection from IP: %s, Port: %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
        log_system_event("Info", towrite2);

        // Allocate socket descriptor for the thread
        int *pclient = malloc(sizeof(int));
        if (pclient == NULL)
        {
            perror("Malloc failed");
            close(new_socket);
            log_system_event("Error", "Malloc failed");
            close_logging(log_file);
            continue;
        }
        *pclient = new_socket;

        // Create a new thread for the client
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, connection_handler, pclient) != 0)
        {
            log_system_event("Error", "Could not create thread");
            close_logging(log_file);
            perror("Could not create thread");
            free(pclient);
            close(new_socket);
            continue;
        }

        // Detach the thread so it cleans up after itself
        pthread_detach(thread_id);
    }

    close(server_fd);

    pthread_cancel(save_thread);
    pthread_mutex_destroy(&trie_mutex);
    saveTrie(root); // Final save
    close_logging(log_file);
    return 0;
}