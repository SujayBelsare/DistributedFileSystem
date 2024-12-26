#include "../headers/clientheader.h"

void handle_error(const char *message, bool fatal)
{
    fprintf(stderr, "Error: %s - %s\n", message, strerror(errno));
    if (fatal)
    {
        exit(1);
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
bool exchange_messages(const char *ip, int port, Message *request, Message *response)
{
    int sock = connect_to_server(ip, port);
    bool success = false;
    if (send(sock, request, sizeof(Message), MSG_NOSIGNAL) < 0)
    {
        handle_error("Send failed", false);
        goto cleanup;
    }

    if (recv(sock, response, sizeof(Message), 0) < 0)
    {
        handle_error("Receive failed", false);
        goto cleanup;
    }
    success = true;

cleanup:
    close(sock);
    return success;
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
        {"WRITE", CMD_WRITE_STATUS},
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