#ifndef CLIENT_H
#define CLIENT_H

#include "../protocols/message.h"
#include "main.h"

void delete_file(Node* child, int client_socket, int flag);
void handle_client(int client_socket, Message *initial_message);
void process_client_request(char *data, size_t size, char sender, struct sockaddr_in *address, int client_socket, int flag);

#endif