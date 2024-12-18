#ifndef TRIE_H
#define TRIE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the maximum length of a file or directory name
#define MAX_NAME 100

#define SAVE_INTERVAL 30

#define CONFIG_FILE "trie_state.conf"
#define BACKUP_FILE "trie_state.conf.bak"

typedef struct Metadata
{
    int number;    // index number for file node (inode)
    int isFile;    // 0 for folder, 1 for file, -1 for other
    int isDeleted; // 0 for _not_ deleted. 1 if deleted.
} Metadata;

// Tree node structure
typedef struct Node
{
    char name[MAX_NAME];
    struct Node *parent;
    struct Node *child;
    struct Node *sibling;
    struct Metadata *metadata;
} Node;

void addChild(Node *parent, const char *name);

void printTree(Node *node, int depth, char* string);

void freeTree(Node *node);

Node *navigateTo(Node *current, const char *name);

Node *navigatePath(Node *current, const char *path);

Node *createNode(const char *name, Node *parent);

void deleteTree(Node* node, int client_socket);

pthread_t startAutoSave(Node *root);

Node *restoreTrie();

void saveTrie(Node *root);

void listAllSubNodes(Node *current, char *list);
#endif