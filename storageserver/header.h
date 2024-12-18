#ifndef _HEADER_H_
#define _HEADER_H_

#include "stdio.h"
#include "netdb.h"
#include "netinet/in.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "sys/socket.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "unistd.h"
#include "arpa/inet.h"
#include "fcntl.h"
#include "dirent.h"
#include "semaphore.h"
#include "pthread.h"
#include "assert.h"
#include "errno.h"
#include "sys/wait.h"
#include "libgen.h"
#include "time.h"
#include "sys/ioctl.h"
#include "pwd.h"
#include "grp.h"
#include "time.h"

#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define LINE    "\033[4m"
#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

#define MAX_CLIENTS 1024
#define MAX_FILE_SIZE 1024
#define MAX_FILE_NAME 256
#define MAX_PATH 1024
#define MAX_COMMAND 1024
#define BUFFER_SIZE 2048

#include "headers/parser.h"
#include "headers/file_handling.h"
#include "headers/handling.h"
#include "headers/nameserver.h"
#include "headers/handle_nm.h"
#include "headers/handle_client.h"

// typedef struct Message
// {
//     char sender;      // who is sending. S : Storage Server. N : Name Server. C : Client
//     int packetNo;     // the current packet number of the data
//     int totalPackets; // the total number of packets the sender is expected to send.
//     int datasize;     // the number of bytes of data the sender is sending (in the data field)
//     char data[2048];  // actual data
// } Message;

#endif