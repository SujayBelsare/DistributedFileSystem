#ifndef HANDLE_CLIENT_H
#define HANDLE_CLIENT_H

#include "../header.h"

void process_client_request(char *data, size_t size, char sender, struct sockaddr_in *address, int client_socket);

#endif