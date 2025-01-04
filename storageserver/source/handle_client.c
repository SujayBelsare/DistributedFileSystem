#include "../header.h"

void process_client_request(char *data, size_t size, char sender, struct sockaddr_in *address, int client_socket)
{
	// Print incoming request information
	printf("Received request from client %s:%d\n",
		   inet_ntoa(address->sin_addr),
		   ntohs(address->sin_port));
	printf("Request data: %s\n", data);

	// Initialize response structure
	Message response;
	memset(&response, 0, sizeof(Message));
	response.sender = 'S';
	response.packetNo = 1;
	response.totalPackets = 1;

	// Validate input data
	if (data == NULL || size == 0)
	{
		snprintf(response.data, sizeof(response.data), "ERR_104: Invalid Command");
		response.datasize = strlen(response.data);
		send(client_socket, &response, sizeof(Message), MSG_NOSIGNAL);
		return;
	}

	// Process command using inputParser
	int inputsize = strlen(data);
	char newinput[inputsize];

	strcpy(newinput, data);

	Message msg_send;
	memset(&msg_send, 0, sizeof(Message));
	msg_send.sender = 'S';
	msg_send.packetNo = 1;
	msg_send.totalPackets = 1;

	char *savePtr;
	char *command = strtok_r(newinput, " ", &savePtr);
	if (command)
	{
		char *path = strtok_r(NULL, " ", &savePtr);
		if (path)
		{
			if (!strcmp(command, "READ"))
			{
				read_file(path, client_socket);
			}
			else if (!strcmp(command, "WRITE"))
			{
				write_file(path, client_socket);
			}
			else if (!strcmp(command, "STREAM"))
			{
				stream_file(path, client_socket); // Call the new function for STREAM
			}
			else if (!strcmp(command, "DETAILS"))
			{
				get_data(path, client_socket);
				// printf("Get - Command: %s, path: %s\n", command, path);
			}
			else
			{
				snprintf(msg_send.data, BUFFER_SIZE, "Incorrect command provided. Please try again.\n");
				msg_send.datasize = strlen(msg_send.data);
				printf("%s", msg_send.data);
				send(client_socket, &msg_send, sizeof(Message), 0);
				memset(&msg_send, 0, sizeof(Message));
			}
		}
		else
		{
			snprintf(msg_send.data, BUFFER_SIZE, "Please provide the path to the file\n" RESET);
			msg_send.datasize = strlen(msg_send.data);
			send(client_socket, &msg_send, sizeof(Message), 0);
			memset(&msg_send, 0, sizeof(Message));
		}
	}
	else
	{
		snprintf(msg_send.data, BUFFER_SIZE, "No command provided.\n" RESET);
		msg_send.datasize = strlen(msg_send.data);
		send(client_socket, &msg_send, sizeof(Message), 0);
		memset(&msg_send, 0, sizeof(Message));
	}

	snprintf(response.data, sizeof(response.data), "OK");
	response.datasize = strlen(response.data);
}
