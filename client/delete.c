#include "delete.h"

int processDeleteResponse(Message *response)
{
    printf("%s\n", response->data);
    return 0;
}