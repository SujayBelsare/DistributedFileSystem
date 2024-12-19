#include "create.h"

int processCreateResponse(Message *response)
{
    printf("%s\n", response->data);
    return 0;
}