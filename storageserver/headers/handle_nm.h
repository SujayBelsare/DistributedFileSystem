#ifndef _HANDLE_NM__
#define _HANDLE_NM__

#include "../header.h"

void process_nm_request(char *data, size_t size, char sender, struct sockaddr_in *address, int client_socket);

#endif