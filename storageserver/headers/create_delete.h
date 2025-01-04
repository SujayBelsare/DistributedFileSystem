#ifndef CREATE_DELETE_H
#define CREATE_DELETE_H

#include "../header.h"

void* create_file(char* name, int socket);
void* delete_file(char* name, int socket);

#endif