#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H
#include "../header.h"

void *request_classifier(void *arg);

void request_handler(int nm_socket, Message *initial_message, void (*process_request)(char *, size_t, char, struct sockaddr_in *, int));

#endif