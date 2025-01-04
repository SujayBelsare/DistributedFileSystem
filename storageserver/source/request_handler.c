#include "../header.h"

void *request_classifier(void *arg)
{
	int nm_socket = *((int *)arg);
	free(arg);

	// Read initial message
	Message initial_message;
	memset(&initial_message, 0, sizeof(Message));
	ssize_t bytes_read = recv(nm_socket, &initial_message, sizeof(Message), 0);
	initial_message.data[initial_message.datasize] = 0;
	if (bytes_read <= 0)
	{
		perror("Failed to read initial message");
		close(nm_socket);
		return NULL;
	}

	char sender = initial_message.sender;
	if (sender == 'C')
	{
		printf(YELLOW "CLIENT REQUEST RECIEVED\n" RESET);
		request_handler(nm_socket, &initial_message, &process_client_request);
	}
	else if (sender == 'N')
	{
		printf(YELLOW "NAMESERVER REQUEST RECIEVED\n" RESET);
		request_handler(nm_socket, &initial_message, &process_nm_request);
	}
	// else if (sender == 'S')
	// {
	// 	printf(YELLOW "STORAGE SERVER REQUEST RECIEVED\n" RESET);
	// 	request_handler(nm_socket, &initial_message, &process_sm_request);
	// }
	else
	{
		fprintf(stderr, "Unknown sender type: %c\n", sender);
	}

	close(nm_socket);
	return NULL;
}

void request_handler(int nm_socket, Message *initial_message, void (*process_request)(char *, size_t, char, struct sockaddr_in *, int))
{
	struct sockaddr_in address;
	socklen_t addrlen = sizeof(address);

	if (getpeername(nm_socket, (struct sockaddr *)&address, &addrlen) < 0)
	{
		perror("Failed to get peer name");
		return;
	}

	// Initialize the RequestBuffer
	RequestBuffer *requestBuffer = init_request_buffer();
	if (!requestBuffer)
	{
		return;
	}

	Message message;
	memset(&message, 0, sizeof(Message));
	memcpy(&message, initial_message, sizeof(Message));

	while (1)
	{
		message.data[2047] = 0;

		if (message.datasize > sizeof(message.data))
		{
			fprintf(stderr, "Invalid data size received from client %s:%d.\n",
					inet_ntoa(address.sin_addr),
					ntohs(address.sin_port));
			break;
		}

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
				break;
			}

			// Optional: Verify that the packetNo is within expected range
			if (message.packetNo < 1 || message.packetNo > requestBuffer->expectedTotal)
			{
				fprintf(stderr, "Received out-of-range packet number %d from client %s:%d.\n",
						message.packetNo,
						inet_ntoa(address.sin_addr),
						ntohs(address.sin_port));
				break;
			}
		}

		if (append_to_request_buffer(requestBuffer, message.data, message.datasize) < 0)
		{
			fprintf(stderr, "Failed to append data to request buffer for client %s:%d.\n",
					inet_ntoa(address.sin_addr),
					ntohs(address.sin_port));
			break;
		}

		requestBuffer->receivedPackets++;
		// If all packets have been received, process the complete request
		if (requestBuffer->receivedPackets >= requestBuffer->expectedTotal)
		{
			// Process the complete request
			process_request(requestBuffer->data, requestBuffer->size, message.sender, &address, nm_socket);

			// Reset the buffer for the next request
			reset_request_buffer(requestBuffer);
			return;
		}

		ssize_t valread = recv(nm_socket, &message, sizeof(Message), MSG_WAITALL);
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
			break;
		}
		else
		{
			// Error occurred during recv
			perror("Receive failed");
			break;
		}
	}
	reset_request_buffer(requestBuffer);
	free(requestBuffer);
}
