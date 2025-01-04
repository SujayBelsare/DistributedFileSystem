#include "../headers/clientheader.h"

int main(void)
{
    const char nm_ip[] = "0.0.0.0";
    const int nm_port = 34000;

    char command[MAX_COMMAND];
    ServerInfo storage_server;

    // Initialize write status mutex
    pthread_mutex_init(&write_status_mutex, NULL);

    while (1)
    {
        char command[DATA_SIZE];
        printf("Enter command (or type EXIT to quit): ");
        memset(command, 0, DATA_SIZE);
        if (!fgets(command, DATA_SIZE, stdin))
        {
            handle_error("Failed to read command", true);
        }
        command[strcspn(command, "\n")] = '\0';

        // Check if user wants to exit
        if (strcmp(command, "EXIT") == 0)
        {
            printf("Exiting client.\n");
            break;
        }
        CommandType cmd_type = parse_command(command);
        if (cmd_type == CMD_UNKNOWN)
        {
            printf("ERROR: INVALID COMMAND\n");
            continue; // Redirect to the `while` loop for re-input
        }

        Message request = {
            .sender = 'C',
            .packetNo = 1,
            .totalPackets = 1,
            .datasize = strlen(command)};

        strncpy(request.data, command, DATA_SIZE - 1);
        request.data[DATA_SIZE - 1] = '\0';

        Message response;
        int NMsocket = exchange_messages(nm_ip, nm_port, &request, &response);
        if (!NMsocket)
        {
            handle_error("Failed to communicate with Naming Server", true);
        }

        switch (cmd_type)
        {
        case CMD_CREATE:
            processCreateResponse(&response);
            break;

        case CMD_READ:
            processReadResponse(&response);
            break;

        case CMD_WRITE:
            processWriteResponse(&response);
            break;

        case CMD_LIST:
            processListResponse(&response);
            break;

        case CMD_DELETE:
            processDeleteResponse(&response);
            break;

        case CMD_DETAILS:
            processGetResponse(&response);
            break;

        case CMD_COPY:
            processCopyResponse(&response, NMsocket);
            break;

        case CMD_STREAM:
            if (!(sscanf(response.data, "%s %d", storage_server.ip, &storage_server.port) == 2))
            {
                handle_error("Failed to parse Nameserver response.\n", true);
                continue;
            }
            stream_music(storage_server.ip, storage_server.port, command);
            break;

        default:
            handle_error("Unsupported command", true);
        }
        close(NMsocket);
    }
    pthread_mutex_destroy(&write_status_mutex);
    return 0;
}
