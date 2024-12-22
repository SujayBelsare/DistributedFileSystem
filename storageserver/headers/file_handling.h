#ifndef _FILE_HANDLING_H_
#define _FILE_HANDLING_H_

#include "../header.h"

void *read_file(char *path, int socket);
void *write_file(char *path, int mode, int socket, char *content);
void *get_data(char *path, int socket);

#endif