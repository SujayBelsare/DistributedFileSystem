#include "../header/main.h"

int copycontents(Node *sourcefile, Node *destinationFile)
{
    if (sourcefile == NULL || destinationFile == NULL)
    {
        return -1;
    }
    pthread_mutex_lock(&shared_data_mutex);
    Connection *sourceMain = fileArray[sourcefile->metadata->number].main;
    Connection *sourceBackup1 = fileArray[sourcefile->metadata->number].backup1;
    Connection *sourceBackup2 = fileArray[sourcefile->metadata->number].backup2;

    Connection *destinationMain = fileArray[destinationFile->metadata->number].main;
    Connection *destinationBackup1 = fileArray[destinationFile->metadata->number].backup1;
    Connection *destinationBackup2 = fileArray[destinationFile->metadata->number].backup2;
    pthread_mutex_unlock(&shared_data_mutex);

    char *main_ip = inet_ntoa(sourceMain->address.sin_addr);
    int main_port = ntohs(sourceMain->address.sin_port);

    Message requests;
    requests.sender = 'N';
    requests.packetNo = 1;
    requests.totalPackets = 1;
    requests.datasize = snprintf(requests.data, DATA_SIZE, "COPY %d %s %d %d", sourcefile->metadata->number, main_ip, main_port, destinationFile->metadata->number);

    // send the copy request to the destination server
    exchangeMessage(inet_ntoa(destinationMain->address.sin_addr), ntohs(destinationMain->address.sin_port), &requests);
    if (sourceBackup1 == NULL || sourceBackup2 == NULL)
    {
        return 0;
    }
    exchangeMessage(inet_ntoa(destinationBackup1->address.sin_addr), ntohs(destinationBackup1->address.sin_port), &requests);
    exchangeMessage(inet_ntoa(destinationBackup2->address.sin_addr), ntohs(destinationBackup2->address.sin_port), &requests);
    return 0;
}