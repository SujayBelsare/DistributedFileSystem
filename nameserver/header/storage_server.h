#ifndef STORAGE_H__
#define STORAGE_H__

#include "../protocols/message.h"
#include "main.h"

void process_server_request(char *data, size_t size, char sender, struct sockaddr_in *address, int client_socket, int flag);

#endif