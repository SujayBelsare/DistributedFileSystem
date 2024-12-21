#ifndef _HEADER_H_
#define _HEADER_H_

#include "stdio.h"
#include "netdb.h"
#include "netinet/in.h"
#include "stdlib.h"
#include "string.h"
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

#include "create.h"
#include "client.h"
#include "write.h"
#include "delete.h"
#include "copy.h"
#include "read.h"
#include "get_data.h"

#endif