#ifndef COMP2310_CACHE_H
#define COMP2310_CACHE_H

#include <stdlib.h>
#include "csapp.h"

#define MAX_CACHE_SIZE 1049000
#define MAX_BLOCK_SIZE 1049


typedef struct CacheBlock CacheBlock;
typedef struct Cache Cache;

struct CacheBlock {
    int size;
    char host[MAXLINE];
    char path[MAXLINE];
    char data[MAX_BLOCK_SIZE];

};

struct Cache {
    CacheBlock* blocks;
    int nblocks;
    int maxblocks;
};

Cache* create_cache();
void add_to_cache(Cache* cache, char *hostname, char *path, char *data, int n);
CacheBlock * get_cached_data(Cache* cache, char *hostname, char *path);


#endif

