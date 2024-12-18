#ifndef PATHRES_H
#define PATHRES_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "trie.h"

void LRU_SETUP();

Node* getNodeFromPath(Node* root, char* path);


#endif