#include "../headers/clientheader.h"

void stream_music(const char *ss_ip, int ss_port, const char *path)
{
    int sock = connect_to_server(ss_ip, ss_port);

    // Send the stream request
    Message message = {
        .sender = 'C',
        .packetNo = 1,
        .totalPackets = 1,
        .datasize = strlen(path)};
    strncpy(message.data, path, DATA_SIZE - 1);
    message.data[DATA_SIZE - 1] = '\0';

    if (send(sock, &message, sizeof(Message), MSG_NOSIGNAL) < 0)
    {
        handle_error("Failed to send stream request", false);
        close(sock);
        return;
    }

    FILE *player = popen("mpv --no-terminal -", "w");
    if (!player)
    {
        handle_error("Failed to start music player", false);
        close(sock);
        return;
    }

    // Receive total packet count
    Message response;
    ssize_t bytes_received = recv(sock, &response, sizeof(Message), 0);
    if (bytes_received <= 0 || response.totalPackets <= 0)
    {
        printf("%s\n", response.data);
        close(sock);
        pclose(player);
        return;
    }

    int total_packets = response.totalPackets;
    printf("Total packets to receive: %d\n", total_packets);

    // Receive and play each packet
    int packets_received = 0;
    while (packets_received < total_packets)
    {
        bytes_received = recv(sock, &response, sizeof(Message), 0);
        if (bytes_received <= 0)
        {
            perror("Error receiving stream data");
            break;
        }

        packets_received++;
        if (fwrite(response.data, 1, response.datasize, player) != response.datasize)
        {
            handle_error("Failed to write to player", false);
            break;
        }
    }

    if (packets_received == total_packets)
    {
        printf("All packets received. Stream complete.\n");
    }
    else
    {
        printf("Stream incomplete. Received %d/%d packets.\n", packets_received, total_packets);
    }

    pclose(player);
    close(sock);
}
