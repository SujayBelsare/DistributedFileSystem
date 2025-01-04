#include "../headers/clientheader.h"

void handle_error(const char *message, bool fatal)
{
    fprintf(stderr, RED "Error: %s\n" RESET, message);
    if (fatal)
    {
        printf(RED "Fatal error detected. Performance may be compromised.\n" RESET);
    }
}

// Create and connect socket
int connect_to_server(const char *ip, int port)
{
    int sock;
    struct sockaddr_in server_addr;
    int retries = 10;
    while (retries-- > 0)
    {
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            handle_error("Socket creation failed", false);
            continue;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0)
        {
            close(sock);
            handle_error("Invalid address", true);
        }

        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == 0)
        {
            return sock;
        }

        close(sock);
    }

    handle_error("Connection failed after retries", true);
    return -1;
}

// Generic function to send/receive messages
int exchange_messages(const char *ip, int port, Message *request, Message *response)
{
    int sock = connect_to_server(ip, port);
    if (send(sock, request, sizeof(Message), MSG_NOSIGNAL) < 0)
    {
        handle_error("Send failed", false);
        close(sock);
        return false;
    }

    if (recv(sock, response, sizeof(Message), 0) < 0)
    {
        handle_error("Receive failed", false);
        close(sock);
        return false;
    }

    return sock;
}

CommandType parse_command(const char *command)
{
    const struct
    {
        const char *cmd;
        CommandType type;
    } commands[] = {
        {"STREAM", CMD_STREAM},
        {"CREATE", CMD_CREATE},
        {"READ", CMD_READ},
        {"DELETE", CMD_DELETE},
        {"COPY", CMD_COPY},
        {"WRITE", CMD_WRITE},
        {"UPLOAD", CMD_UPLOAD},
        {"DETAILS", CMD_DETAILS},
        {"LIST", CMD_LIST},

    };

    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
    {
        if (strncmp(command, commands[i].cmd, strlen(commands[i].cmd)) == 0)
        {
            return commands[i].type;
        }
    }
    return CMD_UNKNOWN;
}