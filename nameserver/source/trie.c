#include "../header/main.h"

// Create a new node
Node *createNode(const char *name, Node *parent)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (!newNode)
    {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    strncpy(newNode->name, name, MAX_NAME - 1);
    newNode->name[MAX_NAME - 1] = '\0';
    newNode->parent = parent;
    newNode->child = NULL;
    newNode->sibling = NULL;

    newNode->metadata = malloc(sizeof(Metadata));
    newNode->metadata->isFile = -1;
    newNode->metadata->number = -1;
    newNode->metadata->isDeleted = 0;
    return newNode;
}

// Add a child to a parent node
void addChild(Node *parent, const char *name)
{
    // Check if the parent is a file
    if (parent->metadata->isFile)
    {
        printf("Cannot create a child under a file.\n");
        return;
    }
    // Check if the child already exists
    Node *child = parent->child;
    while (child)
    {
        if (strcmp(child->name, name) == 0)
        {
            // check if it's deleted
            if (child->metadata->isDeleted)
            {
                child->metadata->isDeleted = 0;
                return;
            }
            printf("Item '%s' already exists under '%s'\n", name, parent->name);
            return;
        }
        child = child->sibling;
    }

    // Create a new node and add it to the parent
    Node *newNode = createNode(name, parent);
    if (!parent->child)
    {
        parent->child = newNode;
    }
    else
    {
        Node *current = parent->child;
        while (current->sibling)
        {
            current = current->sibling;
        }
        current->sibling = newNode;
    }

    return;
}

// Print the tree structure recursively
void printTree(Node *node, int depth, char *string)
{
    if (!string)
    {
        if (!node)
            return;
        if (node->metadata->isDeleted != 1)
        {
            for (int i = 0; i < depth; ++i)
            {
                printf("    ");
            }
            printf("%s\n", node->name);
        }
        printTree(node->child, depth + 1, string);
        printTree(node->sibling, depth, string);
    }
    else
    {
        if (!node)
            return;
        if (node->metadata->isDeleted != 1)
        {
            for (int i = 0; i < depth; ++i)
            {
                strcat(string, "    ");
            }
            strcat(string, node->name);
            strcat(string, "\n");
        }
        printTree(node->child, depth + 1, string);
        printTree(node->sibling, depth, string);
    }
}

// Navigate to a child directory
Node *navigateTo(Node *current, const char *name)
{
    if (current == NULL || current->metadata->isDeleted || current->metadata->isFile)
    {
        return NULL;
    }

    Node *child = current->child;
    while (child)
    {
        if (strcmp(child->name, name) == 0 && !child->metadata->isDeleted)
        {
            return child;
        }
        child = child->sibling;
    }
    printf("Item '%s' not found under '%s'\n", name, current->name);
    return NULL;
}

Node *navigatePath(Node *current, const char *path)
{
    char *saveptr;
    char *token = __strtok_r((char *)path, "/", &saveptr);
    while (token)
    {
        current = navigateTo(current, token);
        if (current == NULL || current->metadata->isDeleted)
        {
            return NULL;
        }
        token = __strtok_r(NULL, "/", &saveptr);
    }
    return current;
}

// Navigate back to parent
Node *navigateBack(Node *current)
{
    if (current->parent)
    {
        return current->parent;
    }
    printf("Already at the root directory.\n");
    return current;
}

// Free the tree recursively
void freeTree(Node *node)
{
    if (!node)
    {
        return;
    }
    freeTree(node->child);
    freeTree(node->sibling);
    free(node->metadata);
    free(node);
}

// Lazy-Delete the tree recursively
void deleteTree(Node *node, int client_socket)
{
    if (!node)
    {
        return;
    }
    deleteTree(node->child, client_socket);
    deleteTree(node->sibling, client_socket);
    node->metadata->isDeleted = 1;
    if (node->metadata->isFile)
    {
        delete_file(node, client_socket);
    }
}

// Save the trie state to a file
void serializeNode(FILE *fp, Node *node)
{
    if (!node)
        return;

    // Write node data
    fprintf(fp, "NODE\n");
    fprintf(fp, "name:%s\n", node->name);
    fprintf(fp, "metadata:%d,%d,%d\n",
            node->metadata->number,
            node->metadata->isFile,
            node->metadata->isDeleted);

    // Mark relationships
    fprintf(fp, "has_child:%d\n", node->child ? 1 : 0);
    fprintf(fp, "has_sibling:%d\n", node->sibling ? 1 : 0);

    // Recursively serialize child and sibling
    if (node->child)
        serializeNode(fp, node->child);
    if (node->sibling)
        serializeNode(fp, node->sibling);
}

void saveTrie(Node *root)
{
    // Create temporary file
    FILE *fp = fopen(BACKUP_FILE, "w");
    if (!fp)
    {
        perror("Failed to create backup file");
        return;
    }

    // Write format version for future compatibility
    fprintf(fp, "VERSION:1.0\n");

    // Serialize the trie
    serializeNode(fp, root);

    fclose(fp);

    // Atomic rename for safe file replacement
    rename(BACKUP_FILE, CONFIG_FILE);
}

Node *deserializeNode(FILE *fp, Node *parent)
{
    char line[MAX_NAME * 2];
    char name[MAX_NAME];
    int number, isFile, isDeleted;
    int has_child, has_sibling;

    // Read node marker
    if (!fgets(line, sizeof(line), fp))
        return NULL;
    if (strncmp(line, "NODE", 4) != 0)
        return NULL;

    // Create new node
    Node *node = malloc(sizeof(Node));
    node->metadata = malloc(sizeof(Metadata));
    node->parent = parent;

    // Read node data
    fscanf(fp, "name:%s\n", name);
    strcpy(node->name, name);

    fscanf(fp, "metadata:%d,%d,%d\n", &number, &isFile, &isDeleted);
    node->metadata->number = number;
    node->metadata->isFile = isFile;
    node->metadata->isDeleted = isDeleted;

    fscanf(fp, "has_child:%d\n", &has_child);
    fscanf(fp, "has_sibling:%d\n", &has_sibling);

    // Recursively restore relationships
    node->child = has_child ? deserializeNode(fp, node) : NULL;
    node->sibling = has_sibling ? deserializeNode(fp, parent) : NULL;

    return node;
}

Node *restoreTrie()
{
    FILE *fp = fopen(CONFIG_FILE, "r");
    if (!fp)
    {
        printf("No existing trie state found\n");
        return NULL;
    }

    char version[20];
    fscanf(fp, "VERSION:%s\n", version);

    // Here you could add version compatibility checks

    Node *root = deserializeNode(fp, NULL);
    fclose(fp);

    return root;
}

extern pthread_mutex_t trie_mutex;

void *autoSaveThread(void *arg)
{
    Node *root = (Node *)arg;

    while (1)
    {
        sleep(SAVE_INTERVAL);

        pthread_mutex_lock(&trie_mutex);
        saveTrie(root);
        pthread_mutex_unlock(&trie_mutex);
    }

    return NULL;
}

pthread_t startAutoSave(Node *root)
{
    pthread_t thread_id;

    pthread_create(&thread_id, NULL, autoSaveThread, root);
    return thread_id;
}

// make a function which returns a string of paths of every single subnode under a current node
// this will be used to send the list of files and directories to the client

void listAllSubNodesWithPath(Node *current, char *list, char *currentPath)
{
    if (!current)
    {
        return;
    }

    // Create temporary buffer for building the current full path
    char newPath[1024] = {0}; // Adjust size as needed

    if (strlen(currentPath) > 0)
    {
        strcpy(newPath, currentPath);
        strcat(newPath, "/");
    }
    strcat(newPath, current->name);

    // Add to list if node is not deleted
    if (current->metadata->isDeleted != 1)
    {
        strcat(list, newPath);
        strcat(list, "\n");
    }

    // Traverse child with the new path
    listAllSubNodesWithPath(current->child, list, newPath);

    // Traverse sibling with the original path
    listAllSubNodesWithPath(current->sibling, list, currentPath);
}

void listAllSubNodes(Node *current, char *list)
{
    listAllSubNodesWithPath(current, list, "");
}