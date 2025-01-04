#ifndef CLIENT_H
#define CLIENT_H

#include "clientheader.h"

typedef enum
{
    CMD_STREAM,
    CMD_CREATE,
    CMD_READ,
    CMD_DELETE,
    CMD_COPY,
    CMD_WRITE,
    CMD_UPLOAD,
    CMD_DETAILS,
    CMD_LIST,
    CMD_UNKNOWN
} CommandType;

void handle_error(const char *message, bool fatal);

int connect_to_server(const char *ip, int port);

int exchange_messages(const char *ip, int port, Message *request, Message *response);

CommandType parse_command(const char *command);

#endif