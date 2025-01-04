#ifndef CONNECTIONS_H
#define CONNECTIONS_H

void *connection_handler(void *arg);

void connection_caller(int client_socket, Message *initial_message, void (*process_request)(char *, size_t, char, struct sockaddr_in *, int, int));

#endif