#include "../header/main.h"

extern Node *root;
extern int server_num;
extern MinHeap *mainHeap;
extern Server *fileArray;
extern Connection *serverArray;

// create a lock
pthread_mutex_t serverArrayMutex = PTHREAD_MUTEX_INITIALIZER;
void process_server_request(char *data, size_t size, char sender, struct sockaddr_in *address, int client_socket, int flag)
{
    char *token;
    char *ip_val = __strtok_r(data, " \n,", &token);
    char *port_val = __strtok_r(NULL, " \n,", &token);
    char *files_val = __strtok_r(NULL, " \n,", &token);
    
    struct sockaddr_in* server_addr = malloc(sizeof(struct sockaddr_in));
    memset(server_addr, 0, sizeof(struct sockaddr_in));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(atoi(port_val));
    
    if (inet_pton(AF_INET, ip_val, &server_addr->sin_addr) <= 0) {
        perror("Invalid Addr / Addr not supported");
        log_system_event("Error", "Invalid Addr / Addr not supported");
        return;
    }
    Connection *new = malloc(sizeof(Connection));
    memset(new, 0, sizeof(sizeof(Connection)));
    new->address = *server_addr;
    new->address.sin_addr = address->sin_addr;
    new->socket = client_socket;
    
    new->filecount = atoi(files_val);
    pthread_mutex_lock(&serverArrayMutex);
    
    new->server_id = server_num;
    serverArray[server_num] = *new;
    server_num++;
    printf("%s %d\n", inet_ntoa(address->sin_addr), ntohs(new->address.sin_port));
    char toWrite[BUFSIZ];
    snprintf(toWrite, BUFSIZ, "New server connected: IP %s, Port %d\n", inet_ntoa(address->sin_addr), ntohs(new->address.sin_port));
    log_system_event("Info", toWrite);
    insert(mainHeap, new);
    
    pthread_mutex_unlock(&serverArrayMutex);
    
    Message response;
    memset(&response, 0, sizeof(Message));
    response.sender = 'N';
    response.packetNo = 1;
    response.totalPackets = 1;
    snprintf(response.data, sizeof(response.data), "OK");
    
    response.datasize = strlen(response.data);
    send(client_socket, &response, sizeof(Message), 0);
    return;
}
