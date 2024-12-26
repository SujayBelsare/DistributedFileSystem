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

        // Validate command type
        int valid_command = (strncmp(command, "CREATE", 6) == 0 ||
                             strncmp(command, "READ", 4) == 0 ||
                             strncmp(command, "STREAM", 6) == 0 ||
                             strncmp(command, "WRITE", 5) == 0 ||
                             strncmp(command, "DELETE", 6) == 0 ||
                             strncmp(command, "LIST", 4) == 0 ||
                             strncmp(command, "COPY", 4) == 0 ||
                             strncmp(command, "DETAILS", 7) == 0);

        if (!valid_command)
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
        if (!exchange_messages(nm_ip, nm_port, &request, &response))
        {
            handle_error("Failed to communicate with Naming Server", true);
        }

        // Check for specific error codes in the response
        if (strncmp(response.data, "1001:", 5) == 0 || // Directory inside file
            strncmp(response.data, "1002:", 5) == 0 || // Invalid path
            strncmp(response.data, "1004:", 5) == 0 || // Resource already exists
            strncmp(response.data, "ERROR:", 6) == 0)  // Generic error
        {
            printf("Server Error: %s\n", response.data);
            continue; // Redirect to the `while` loop for re-input
        }

        printf("NM Response: %s\n", response.data);

        CommandType cmd_type = parse_command(command);
        switch (cmd_type)
        {
        case CMD_STREAM:
            if (!(sscanf(response.data, "%s %d", storage_server.ip, &storage_server.port) == 2))
            {
                printf("Incorrect input, please try again.\n");
                continue;
            }
            stream_music(storage_server.ip, storage_server.port, command);
            break;

        case CMD_CREATE:
            processCreateResponse(&response);
            break;
        case CMD_READ:
            processReadResponse(&response);
            break;
        case CMD_LIST:
            processListResponse(&response);
            break;
        case CMD_WRITE_STATUS:
            printf("Command: %s\n", command);
            char temp1[50]; // Allocate memory for temp1
            char temp2[50]; // Allocate memory for temp2
            char temp3[50]; // Allocate memory for temp3
            char temp4[50]; // Allocate memory for temp4
            char content[1024 * 1024];
            int write_kind;
            // the current format is WRITE FILE <file_path> <file_name> <write_kind> <content>
            sscanf(command, "%s %s %s %s %d %[^\n]s", temp1, temp2, temp3, temp4, &write_kind, content);
            // printf("content:%s\n", content);
            size_t content_len = strlen(content);
            size_t total_chunks = (content_len + 2000 - 1) / 2000;
            size_t bytes_sent = 0;

            char content_to_be_sent[2048];
            strncpy(content_to_be_sent, content + bytes_sent, 2000);
            processWriteResponse(&response, content_to_be_sent, write_kind);
            int chunks = 1;
            bytes_sent += strlen(content_to_be_sent);
            while (chunks < total_chunks)
            {
                strncpy(content_to_be_sent, content + bytes_sent, 2000);
                processWriteResponse(&response, content_to_be_sent, 1);
                bytes_sent += strlen(content_to_be_sent);
            }
            break;
            // processReadResponse(&response);
        case CMD_DELETE:
            processDeleteResponse(&response);
            break;
        case CMD_COPY:
            //int processCopyResponse(Message *initial_respose, char *nmip, int nmport)
        case CMD_DETAILS:
            // syntax: DETAILS <file_path>
            processGetResponse(&response);
            break;
        default:
            handle_error("Unsupported command", true);
        }
    }
    pthread_mutex_destroy(&write_status_mutex);
    return 0;
}
