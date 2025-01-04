#ifndef GET_READ_WRITE_H
#define GET_READ_WRITE_H

#include "../header.h"

void *read_file(char *path, int socket);
void *write_file(char *path, int socket);
void *get_data(char *path, int socket);

#endif