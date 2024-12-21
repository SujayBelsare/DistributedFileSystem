#include "../header/main.h"
extern Node *LRU_CACHE[HASHMAP_SIZE];

void LRU_SETUP()
{
    memset(LRU_CACHE, 0, sizeof(LRU_CACHE));
    return;
}

int chartoint(char c){
    if(c >= '0' && c <= '9'){
        return c - '0' + 1; // 1-10
    }
    else if(c >= 'A'  && c <= 'Z'){
        return c - 'A' + 11; // 11-36
    }
    else if(c >= 'a' && c <= 'z'){
        return c - 'a' + 37; // 37-62
    }
    else if(c=='.' || c == '/')
    {
        return 1;
    }
    return 0;
}

int prime = 98317;
int getHash(char *string)
{
    int index = 0;
    int currHash = 1;
    int curr_char = chartoint(string[index]);

    int base = 63;

    while (curr_char != '\0')
    {
        currHash = (currHash * base + chartoint(string[index])) % prime;
        index += 1;
        curr_char = chartoint(string[index]);
    }
    return currHash;
}

pthread_mutex_t cache_mutex = PTHREAD_MUTEX_INITIALIZER;
Node *getNodeFromPath(Node *root, char *path)
{
    printf("%s\n",path);
    printf("DEBUG-GNFP-1\n");
    int pathHash = getHash(path);
    printf("path hash:%d\n", pathHash);
    if((LRU_CACHE[pathHash] != NULL && LRU_CACHE[pathHash]->metadata->isDeleted == 0))
    {
        // verify if the path is the same
        // if(strcmp(LRU_CACHE[pathHash]->name, path) == 0)
        // {
        return LRU_CACHE[pathHash];
        // }
    }
    else
    {
        // path not found in cache
        // search in the tree
        Node *node = navigatePath(root, path);
        if (node != NULL && node->metadata->isDeleted == 0)
        {
            // lock the cache
            pthread_mutex_lock(&cache_mutex);
            LRU_CACHE[pathHash] = node;
            pthread_mutex_unlock(&cache_mutex);
            return node;
        }
    }

    return NULL;
}