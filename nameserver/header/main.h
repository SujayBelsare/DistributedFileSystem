#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // close()
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // inet_ntoa()
#include <pthread.h>
#include <sys/select.h>

#include "../protocols/message.h"
#include "../protocols/identifiers.h"

#include "trie.h"
#include "handle-client.h"
#include "connections.h"
#include "storage_server.h"
#include "pathres.h"
#include "bookkeeping.h"
#include "delete.h"

#define SO_REUSEPORT 15

#define MAX_CLIENTS 10000
#define CACHE_SIZE 25
#define HASHMAP_SIZE 98317

char *exchangeMessage(const char *ip, int port, Message *request);
#endif