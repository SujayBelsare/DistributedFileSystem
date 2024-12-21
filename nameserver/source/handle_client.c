#include "../header/main.h"

extern Node *root;
extern int curr_num;
extern MinHeap *mainHeap;
extern Server *fileArray;
extern Connection *serverArray;
extern FILE *log_file;

pthread_mutex_t shared_data_mutex = PTHREAD_MUTEX_INITIALIZER;

void handle_client(int client_socket, Message *initial_message)
{
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    if (getpeername(client_socket, (struct sockaddr *)&address, &addrlen) < 0)
    {
        perror("Failed to get peer name");
        log_system_event("Error", "Failed to get peer name");
        return;
    }

    // Initialize the RequestBuffer
    RequestBuffer *requestBuffer = init_request_buffer();
    if (!requestBuffer)
    {
        return;
    }

    Message message;
    memset(&message, 0, sizeof(message));
    memcpy(&message, initial_message, sizeof(Message));

    while (1)
    {
        message.data[2047] = 0;
        if (message.datasize > sizeof(message.data))
        {
            fprintf(stderr, "Invalid data size received from client %s:%d.\n",
                    inet_ntoa(address.sin_addr),
                    ntohs(address.sin_port));
            char toWrite[BUFSIZ];
            snprintf(toWrite, BUFSIZ, "Invalid data size received from client %s:%d.\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            log_system_event("Error", toWrite);
            break;
        }

        // If it's the first packet of a new request, initialize the buffer
        if (message.packetNo == 1)
        {
            reset_request_buffer(requestBuffer);
            requestBuffer->expectedTotal = message.totalPackets;
            requestBuffer->receivedPackets = 0;
        }
        else
        {
            // Ensure that we're in the middle of assembling a request
            if (requestBuffer->expectedTotal == 0)
            {
                fprintf(stderr, "Received packet %d without starting a new request from client %s:%d.\n",
                        message.packetNo,
                        inet_ntoa(address.sin_addr),
                        ntohs(address.sin_port));
                char toWrite[BUFSIZ];
                snprintf(toWrite, BUFSIZ, "Received packet %d without starting a new request from client %s:%d.\n", message.packetNo, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                log_system_event("Error", toWrite);
                break;
            }

            // Optional: Verify that the packetNo is within expected range
            if (message.packetNo < 1 || message.packetNo > requestBuffer->expectedTotal)
            {
                fprintf(stderr, "Received out-of-range packet number %d from client %s:%d.\n",
                        message.packetNo,
                        inet_ntoa(address.sin_addr),
                        ntohs(address.sin_port));
                char toWrite[BUFSIZ];
                snprintf(toWrite, BUFSIZ, "Received out-of-range packet number %d from client %s:%d.\n", message.packetNo, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                log_system_event("Error", toWrite);
                break;
            }
        }
        // Append the data from the current packet to the request buffer
        if (append_to_request_buffer(requestBuffer, message.data, message.datasize) < 0)
        {
            fprintf(stderr, "Failed to append data to request buffer for client %s:%d.\n",
                    inet_ntoa(address.sin_addr),
                    ntohs(address.sin_port));
            char toWrite[BUFSIZ];
            snprintf(toWrite, BUFSIZ, "Failed to append data to request buffer for client %s:%d.\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            log_system_event("Error", toWrite);
            break;
        }
        requestBuffer->receivedPackets++;

        // If all packets have been received, process the complete request
        if (requestBuffer->receivedPackets >= requestBuffer->expectedTotal)
        {

            // Process the complete request
            process_client_request(requestBuffer->data, requestBuffer->size, message.sender, &address, client_socket);

            // Reset the buffer for the next request
            reset_request_buffer(requestBuffer);
        }

        // Read the next message
        ssize_t valread = recv(client_socket, &message, sizeof(Message), MSG_WAITALL);
        if (valread > 0)
        {
            continue; // Continue processing the next message
        }
        else if (valread == 0)
        {
            // Connection closed by the client
            printf("Client disconnected: IP %s, Port %d\n",
                   inet_ntoa(address.sin_addr),
                   ntohs(address.sin_port));
            char toWrite[BUFSIZ];
            snprintf(toWrite, BUFSIZ, "Client disconnected: IP %s, Port %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            log_system_event("Info", toWrite);
            break;
        }
        else
        {
            // Error occurred during recv
            perror("Receive failed");
            log_system_event("Error", "Receive failed");
            break;
        }
    }

    // Clean up
    reset_request_buffer(requestBuffer);
    free(requestBuffer);
}

void process_client_request(char *data, size_t size, char sender, struct sockaddr_in *address, int client_socket)
{
    char *token;
    char *tok1 = __strtok_r(data, " \n", &token);
    char *tok2 = __strtok_r(NULL, " \n", &token);

    if (strcmp(tok1, "CREATE") == 0)
    {
        /**
        CREATE
        syntax:
        - CREATE FILE <PATH> <NAME>
        - CREATE DIR <PATH> <NAME>
        */
        printTree(root, 10, NULL);
        if (strcmp(tok2, "FILE") == 0)
        {
            // Create file
            char *path = __strtok_r(NULL, " \n", &token);

            Node *node = getNodeFromPath(root, path);
            if (node == NULL)
            {
                printf("Invalid path\n");
                log_system_event("Error", "Invalid path");
                Message response;
                response.sender = 'N';
                response.packetNo = 1;
                response.totalPackets = 1;
                snprintf(response.data, sizeof(response.data), "PATH DOES NOT EXIST");
                response.datasize = strlen(response.data);
                send(client_socket, &response, sizeof(Message), 0);
                return;
            }

            char *name = __strtok_r(NULL, " \n", &token);
            if (name == NULL)
            {
                printf("Invalid name\n");
                log_system_event("Error", "Invalid name");
                Message response;
                response.sender = 'N';
                response.packetNo = 1;
                response.totalPackets = 1;
                snprintf(response.data, sizeof(response.data), "INVALID FILENAME.");
                response.datasize = strlen(response.data);
                send(client_socket, &response, sizeof(Message), 0);
                return;
            }

            // check if the root already exists
            Node *checkChild = navigateTo(node, name);
            if (checkChild != NULL)
            {
                printf("File already exists\n");
                log_system_event("Error", "File already exists");
                Message response;
                response.sender = 'N';
                response.packetNo = 1;
                response.totalPackets = 1;
                snprintf(response.data, sizeof(response.data), "FILE ALREADY EXISTS.");
                response.datasize = strlen(response.data);
                send(client_socket, &response, sizeof(Message), 0);
                return;
            }

            pthread_mutex_lock(&shared_data_mutex);
            addChild(node, name);
            Node *child = navigateTo(node, name);
            if (child == NULL)
            {
                pthread_mutex_unlock(&shared_data_mutex);
                printf("Cannot create a file under a file.\n");
                log_system_event("Error", "Cannot create a file under a file.");
                Message response;
                response.sender = 'N';
                response.packetNo = 1;
                response.totalPackets = 1;
                snprintf(response.data, sizeof(response.data), "CANNOT CREATE FILE UNDER A FILE.");
                response.datasize = strlen(response.data);
                send(client_socket, &response, sizeof(Message), 0);

                return;
            }

            child->metadata->isFile = 1;
            child->metadata->number = curr_num + 1;
            child->metadata->isDeleted = 0;
            curr_num++;
            Connection *main = extractMin(mainHeap);    // Extract the main connection
            Connection *backup1 = extractMin(mainHeap); // Extract the first backup connection
            Connection *backup2 = extractMin(mainHeap); // Extract the second backup connection
            char *main_ip = "No server is defined\n";
            int main_port = 0;
            Message mainreq;
            Message backreq1;
            Message backreq2;

            // backup only when more than 2 servers are defined
            if (main == NULL && backup1 == NULL && backup2 == NULL)
            {
                printf("No server defined.\n");
                log_system_event("Error", "No server defined");
                Message response;
                response.sender = 'N';
                response.packetNo = 1;
                response.totalPackets = 1;
                snprintf(response.data, sizeof(response.data), "No server is defined.");
                response.datasize = strlen(response.data);
                send(client_socket, &response, sizeof(Message), 0);
                return;
            }
            else if (main != NULL)
            {
                main->filecount++;
                fileArray[child->metadata->number].main = main;

                insert(mainHeap, main); // Insert the main connection back into the heap
                main_ip = inet_ntoa(main->address.sin_addr);
                main_port = ntohs(main->address.sin_port);

                mainreq.sender = 'N';
                mainreq.packetNo = 1;
                mainreq.totalPackets = 1;
                mainreq.datasize = snprintf(mainreq.data, DATA_SIZE, "CREATE %d", child->metadata->number);
            }
            if (backup1 != NULL && backup2 != NULL)
            {
                backup1->filecount++;
                backup2->filecount++;

                fileArray[child->metadata->number].backup1 = backup1;
                fileArray[child->metadata->number].backup2 = backup2;
                insert(mainHeap, backup1); // Insert the first backup connection back into the heap
                insert(mainHeap, backup2); // Insert the second backup connection back into the heap
                backreq1.sender = 'N';
                backreq1.packetNo = 1;
                backreq1.totalPackets = 1;
                backreq1.datasize = snprintf(backreq1.data, DATA_SIZE, "CREATE %d", child->metadata->number);

                backreq2.sender = 'N';
                backreq2.packetNo = 1;
                backreq2.totalPackets = 1;
                backreq2.datasize = snprintf(backreq2.data, DATA_SIZE, "CREATE %d", child->metadata->number);
            }
            pthread_mutex_unlock(&shared_data_mutex);

            exchangeMessage(main_ip, main_port, &mainreq);

            if (backup1 != NULL && backup2 != NULL)
            {
                exchangeMessage(inet_ntoa(backup1->address.sin_addr), ntohs(backup1->address.sin_port), &backreq1);
                exchangeMessage(inet_ntoa(backup2->address.sin_addr), ntohs(backup2->address.sin_port), &backreq2);
            }

            Message response;
            response.sender = 'N';
            response.packetNo = 1;
            response.totalPackets = 1;
            snprintf(response.data, sizeof(response.data), "FILE CREATED SUCCESSFULLY.");
            log_system_event("Info", "File created successfully");
            response.datasize = strlen(response.data);
            send(client_socket, &response, sizeof(Message), 0);
        }
        else if (strcmp(tok2, "DIR") == 0)
        {
            // Create directory
            char *path = __strtok_r(NULL, " \n", &token);
            Node *node = navigatePath(root, path);
            if (node == NULL)
            {
                printf("Invalid path\n");
                log_system_event("Error", "Invalid path");
                return;
            }
            char *name = __strtok_r(NULL, " \n", &token);
            if (name == NULL)
            {
                printf("Invalid name\n");
                log_system_event("Error", "Invalid name");
                return;
            }

            pthread_mutex_lock(&shared_data_mutex);
            addChild(node, name);
            Node *child = navigateTo(node, name);
            child->metadata = malloc(sizeof(Metadata));
            child->metadata->isFile = 0;
            pthread_mutex_unlock(&shared_data_mutex);
            Message response;
            response.sender = 'N';
            response.packetNo = 1;
            response.totalPackets = 1;
            snprintf(response.data, sizeof(response.data), "THE FOLDER HAS BEEN CREATED SUCCESSFULLY.");
            log_system_event("Info", "Folder created successfully");
            response.datasize = strlen(response.data);

            send(client_socket, &response, sizeof(Message), 0);
        }
        else
        {
            printf("Invalid CREATE command\n");
            log_system_event("Error", "Invalid CREATE command");
        }
    }
    // READ command
    // syntax: READ <PATH>
    else if (strcmp(tok1, "READ") == 0)
    {
        if (tok2 == NULL)
        {
            printf("Invalid READ command: Path missing\n");
            log_system_event("Error", "Invalid READ command: Path missing");
            return;
        }

        // Check if the path exists in the Trie
        Node *node = navigatePath(root, tok2);
        if (node == NULL || node->metadata->isFile == 0)
        {
            printf("File not found or is not a valid file: %s\n", tok2);
            log_system_event("Error", "File not found or is not a valid file");
            Message response;
            response.sender = 'N';
            response.packetNo = 1;
            response.totalPackets = 1;
            snprintf(response.data, sizeof(response.data), "3ERROR: File not found or invalid");
            response.datasize = strlen(response.data);
            send(client_socket, &response, sizeof(Message), 0);
            return;
        }

        // Fetch the main Storage Server handling the file
        pthread_mutex_lock(&shared_data_mutex);
        Connection *main = fileArray[node->metadata->number].main;
        pthread_mutex_unlock(&shared_data_mutex);

        // Send the Storage Server IP and Port to the client
        Message response;
        char *main_ip = inet_ntoa(main->address.sin_addr);
        int main_port = ntohs(main->address.sin_port);
        response.sender = 'N';
        response.packetNo = 1;
        response.totalPackets = 1;
        snprintf(response.data, sizeof(response.data), "%s %d", main_ip, main_port);
        response.datasize = strlen(response.data);

        send(client_socket, &response, sizeof(Message), 0);
    }
    else if (strcmp(tok1, "STREAM") == 0)
    {
        /**
         STREAM
         syntax:
         - STREAM <path>
        */
        if (tok2 == NULL)
        {
            printf("Invalid STREAM command: Path missing\n");
            log_system_event("Error", "Invalid STREAM command: Path missing");
            return;
        }

        // Check if the path exists in the Trie
        Node *node = navigatePath(root, tok2);
        if (node == NULL || node->metadata->isFile == 0)
        {
            printf("File not found or is not a valid file: %s\n", tok2);
            log_system_event("Error", "File not found or is not a valid file");
            Message response;
            response.sender = 'N';
            response.packetNo = 1;
            response.totalPackets = 1;
            snprintf(response.data, sizeof(response.data), "ERROR: File not found or invalid");
            response.datasize = strlen(response.data);
            send(client_socket, &response, sizeof(Message), 0);
            return;
        }

        // Fetch the main Storage Server handling the file
        pthread_mutex_lock(&shared_data_mutex);
        // Connection *main = fileArray[node->metadata->number].main;

        Connection *main = &serverArray[0]; // Extract the main connection
        pthread_mutex_unlock(&shared_data_mutex);

        // Send the Storage Server IP and Port to the client
        Message response;
        char *main_ip = inet_ntoa(main->address.sin_addr);
        int main_port = ntohs(main->address.sin_port);
        response.sender = 'N';
        response.packetNo = 1;
        response.totalPackets = 1;
        printf("%s %d\n", main_ip, main_port);
        snprintf(response.data, sizeof(response.data), "%s %d", main_ip, main_port);
        response.datasize = strlen(response.data);

        send(client_socket, &response, sizeof(Message), 0);
    }
    else if (strcmp(tok1, "DELETE") == 0)
    {
        /**
        DELETE
        syntax:
        - DELETE FILE <PATH> <NAME>
        - DELETE DIR <PATH> <NAME>
        */
        if (strcmp(tok2, "FILE") == 0)
        {

            char *path = __strtok_r(NULL, " \n", &token);
            Node *node = getNodeFromPath(root, path);
            if (node == NULL)
            {
                printf("Invalid path\n");
                log_system_event("Error", "Invalid path");
                Message response;
                response.sender = 'N';
                response.packetNo = 1;
                response.totalPackets = 1;
                snprintf(response.data, sizeof(response.data), "ERROR: INVALID PATH");
                response.datasize = strlen(response.data);
                send(client_socket, &response, sizeof(Message), 0);
                return;
            }
            char *name = __strtok_r(NULL, " \n", &token);
            if (name == NULL)
            {
                printf("Invalid name\n");
                log_system_event("Error", "Invalid name");
                Message response;
                response.sender = 'N';
                response.packetNo = 1;
                response.totalPackets = 1;
                snprintf(response.data, sizeof(response.data), "ERROR: INVALID FILENAME");
                response.datasize = strlen(response.data);
                send(client_socket, &response, sizeof(Message), 0);
                return;
            }
            Node *child = navigateTo(node, name);
            if (child == NULL)
            {
                printf("File not found\n");
                log_system_event("Error", "File not found");
                Message response;
                response.sender = 'N';
                response.packetNo = 1;
                response.totalPackets = 1;
                snprintf(response.data, sizeof(response.data), "ERROR: FILE NOT FOUND");
                response.datasize = strlen(response.data);
                send(client_socket, &response, sizeof(Message), 0);
                return;
            }

            if (child->metadata->isFile == 0)
            {
                printf("Cannot delete a directory\n");
                log_system_event("Error", "Cannot delete a directory");
                Message response;
                response.sender = 'N';
                response.packetNo = 1;
                response.totalPackets = 1;
                snprintf(response.data, sizeof(response.data), "ERROR: CANNOT DELETE A DIRECTORY");
                response.datasize = strlen(response.data);
                send(client_socket, &response, sizeof(Message), 0);
                return;
            }

            pthread_mutex_lock(&shared_data_mutex);
            if (child->metadata->isDeleted == 1)
            {
                pthread_mutex_unlock(&shared_data_mutex);
                printf("File already deleted\n");
                log_system_event("Error", "File already deleted");
                Message response;
                response.sender = 'N';
                response.packetNo = 1;
                response.totalPackets = 1;
                snprintf(response.data, sizeof(response.data), "ERROR: FILE ALREADY DELETED");
                response.datasize = strlen(response.data);
                send(client_socket, &response, sizeof(Message), 0);
                return;
            }
            pthread_mutex_unlock(&shared_data_mutex);

            delete_file(child, client_socket, 1);
            pthread_mutex_lock(&shared_data_mutex);
            child->metadata->isDeleted = 1;
            pthread_mutex_unlock(&shared_data_mutex);
        }
        else if (strcmp(tok2, "DIR") == 0)
        {
            // TODO
            char *path = __strtok_r(NULL, " \n", &token);
            Node *node = getNodeFromPath(root, path);
            if (node == NULL)
            {
                printf("Invalid path\n");
                log_system_event("Error", "Invalid path");
                Message response;
                response.sender = 'N';
                response.packetNo = 1;
                response.totalPackets = 1;
                snprintf(response.data, sizeof(response.data), "ERROR: INVALID PATH");
                response.datasize = strlen(response.data);
                send(client_socket, &response, sizeof(Message), 0);
                return;
            }
            char *name = __strtok_r(NULL, " \n", &token);
            if (name == NULL)
            {
                printf("Invalid name\n");
                log_system_event("Error", "Invalid name");
                Message response;
                response.sender = 'N';
                response.packetNo = 1;
                response.totalPackets = 1;
                snprintf(response.data, sizeof(response.data), "ERROR: INVALID DIRECTORY NAME");
                response.datasize = strlen(response.data);
                send(client_socket, &response, sizeof(Message), 0);
                return;
            }
            Node *child = navigateTo(node, name);
            if (child == NULL)
            {
                printf("Directory not found\n");
                log_system_event("Error", "Directory not found");
                Message response;
                response.sender = 'N';
                response.packetNo = 1;
                response.totalPackets = 1;
                snprintf(response.data, sizeof(response.data), "ERROR: DIRECTORY NOT FOUND");
                response.datasize = strlen(response.data);
                send(client_socket, &response, sizeof(Message), 0);
                return;
            }
            if (child->metadata->isFile == 1)
            {
                printf("Cannot delete a file\n");
                log_system_event("Error", "Cannot delete a file");
                Message response;
                response.sender = 'N';
                response.packetNo = 1;
                response.totalPackets = 1;
                snprintf(response.data, sizeof(response.data), "ERROR: CANNOT DELETE A FILE");
                response.datasize = strlen(response.data);
                send(client_socket, &response, sizeof(Message), 0);
                return;
            }
            deleteTree(child, client_socket);
            Message response;
            response.sender = 'N';
            response.packetNo = 1;
            response.totalPackets = 1;
            strcpy(response.data, "DIRECTORY DELETED SUCCESSFULLY");
            response.datasize = strlen(response.data);

            send(client_socket, &response, sizeof(Message), 0);
        }
    }
    else if (strcmp(tok1, "WRITE") == 0)
    {
        if (strcmp(tok2, "FILE") == 0)
        {

            char *path = __strtok_r(NULL, " \n", &token);
            Node *node = getNodeFromPath(root, path);

            if (node == NULL)
            {
                printf("Invalid path\n");
                log_system_event("Error", "Invalid path");
                return;
            }
            char *name = __strtok_r(NULL, " \n", &token);

            if (name == NULL)
            {
                printf("Invalid name\n");
                log_system_event("Error", "Invalid name");
                return;
            }
            Node *child = navigateTo(node, name);
            if (child->metadata->isFile == 1)
            {
                pthread_mutex_lock(&shared_data_mutex);
                Connection *main = fileArray[child->metadata->number].main;
                Connection *backup1 = fileArray[child->metadata->number].backup1;
                Connection *backup2 = fileArray[child->metadata->number].backup2;
                char *main_ip = "No server is defined\n";
                int main_port = 0;
                if (main == NULL && backup1 == NULL && backup2 == NULL)
                {
                    printf("No server defined.\n");
                    log_system_event("Error", "No server defined");
                    return;
                }
                else if (main != NULL)
                {
                    main->filecount++;
                    fileArray[child->metadata->number].main = main;

                    main_ip = inet_ntoa(main->address.sin_addr);
                    main_port = ntohs(main->address.sin_port);
                }
                if (backup1 != NULL && backup2 != NULL)
                {
                    backup1->filecount++;
                    backup2->filecount++;

                    fileArray[child->metadata->number].backup1 = backup1;
                    fileArray[child->metadata->number].backup2 = backup2;
                }
                pthread_mutex_unlock(&shared_data_mutex);
                // char *content = __strtok_r(NULL, " \n", &token);
                Message response;
                response.sender = 'N';
                response.packetNo = 1;
                response.totalPackets = 1;
                if (backup1 == NULL || backup2 == NULL)
                {
                    // send only main server IP and port
                    snprintf(response.data, sizeof(response.data), "1%s %d %d", main_ip, main_port, child->metadata->number);
                    log_server_response(log_file, inet_ntoa(address->sin_addr), inet_ntoa(main->address.sin_addr), "WRITE", "SUCCESS");
                }
                else
                {
                    snprintf(response.data, sizeof(response.data), "2%s %d %d %s %d %d %s %d %d",
                             main_ip, main_port, child->metadata->number,
                             inet_ntoa(backup1->address.sin_addr), ntohs(backup1->address.sin_port), child->metadata->number,
                             inet_ntoa(backup2->address.sin_addr), ntohs(backup2->address.sin_port), child->metadata->number);
                    log_server_response(log_file, inet_ntoa(address->sin_addr), inet_ntoa(main->address.sin_addr), "WRITE", "SUCCESS");
                }
                response.datasize = strlen(response.data);
                send(client_socket, &response, sizeof(Message), 0);
            }
        }
    }
    else if (strcmp(tok1, "DETAILS") == 0)
    {

        Node *node = getNodeFromPath(root, tok2);

        if (node == NULL || node->metadata->isFile == 0 || node->metadata->isDeleted == 1)
        {
            printf("Invalid path\n");
            log_system_event("Error", "Invalid path");
            Message response;
            response.sender = 'N';
            response.packetNo = 1;
            response.totalPackets = 1;
            snprintf(response.data, sizeof(response.data), "THE GIVEN PATH IS EITHER INVALID OR IS A FOLDER.");
            response.datasize = strlen(response.data);
            send(client_socket, &response, sizeof(Message), 0);

            return;
        }

        if (node->metadata->isFile == 1)
        {
            pthread_mutex_lock(&shared_data_mutex);
            Connection *main = fileArray[node->metadata->number].main;
            Connection *backup1 = fileArray[node->metadata->number].backup1;
            Connection *backup2 = fileArray[node->metadata->number].backup2;
            char *main_ip = NULL;
            int main_port = 0;
            if (main == NULL && backup1 == NULL && backup2 == NULL)
            {
                printf("No server defined.\n");
                log_system_event("Error", "No server defined");
                pthread_mutex_unlock(&shared_data_mutex);
                Message response;
                response.sender = 'N';
                response.packetNo = 1;
                response.totalPackets = 1;
                snprintf(response.data, sizeof(response.data), "NO SERVER DEFINED.");
                response.datasize = strlen(response.data);
                send(client_socket, &response, sizeof(Message), 0);
                return;
            }
            else if (main != NULL)
            {
                main->filecount++;
                fileArray[node->metadata->number].main = main;

                main_ip = inet_ntoa(main->address.sin_addr);
                main_port = ntohs(main->address.sin_port);
            }
            if (backup1 != NULL && backup2 != NULL)
            {
                backup1->filecount++;
                backup2->filecount++;

                fileArray[node->metadata->number].backup1 = backup1;
                fileArray[node->metadata->number].backup2 = backup2;
            }
            pthread_mutex_unlock(&shared_data_mutex);

            printf("DEBUG-DETAILS-4\n");

            Message response;
            response.sender = 'N';
            response.packetNo = 1;
            response.totalPackets = 1;
            if (backup1 == NULL || backup2 == NULL)
            {
                printf("DEBUG-HERE-1\n");
                // send only main server IP and port
                snprintf(response.data, sizeof(response.data), "1%s %d %d", main_ip, main_port, node->metadata->number);
                printf("DEBUG-HERE-2\n");
            }
            else
            {
                snprintf(response.data, sizeof(response.data), "2%s %d %d %s %d %d %s %d %d",
                         main_ip, main_port, node->metadata->number,
                         inet_ntoa(backup1->address.sin_addr), ntohs(backup1->address.sin_port), node->metadata->number,
                         inet_ntoa(backup2->address.sin_addr), ntohs(backup2->address.sin_port), node->metadata->number);
                log_server_response(log_file, inet_ntoa(address->sin_addr), inet_ntoa(main->address.sin_addr), "DETAILS", "SUCCESS");
            }
            response.datasize = strlen(response.data);
            printf("DEBUG-DETAILS-4.5\n");
            send(client_socket, &response, sizeof(Message), 0);
            printf("DEBUG-DETAILS-5\n");
        }
    }
    printTree(root, 10, NULL);
}