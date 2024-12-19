#ifndef _BOOKKEEPING_H_
#define _BOOKKEEPING_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_FILE "nfs_operations.log"
#define MAX_LOG_SIZE 2048


void init_logging(FILE* log_file);
void close_logging(FILE* log_file);
void log_client_request(FILE* log_file, const char* client_ip, const char* port, const char* operation, const char* status);    
void log_server_response(FILE* log_file, const char* client_ip, const char* port, const char* operation, const char* status);
void log_system_event(const char* event, const char* description);

#endif