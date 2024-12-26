#ifndef COPY_H
#define COPY_H

#include "clientheader.h"

int processCopyResponse(Message *initial_respose, char *nmip, int nmport);
int processListResponse(Message *response);

#endif