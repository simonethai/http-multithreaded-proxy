#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csapp.h"
#include "cache.h"

//This cache uses random replacement 

//initialise cache 
Cache* create_cache() {
    printf("create cache\n");
    Cache *cache = (Cache *)malloc(sizeof(Cache));
    CacheBlock* blocksptr = (CacheBlock *)malloc(sizeof(CacheBlock)*(MAX_CACHE_SIZE/MAX_BLOCK_SIZE));
    cache->blocks = blocksptr;
    cache->nblocks = 0;
    cache->maxblocks = MAX_CACHE_SIZE/MAX_BLOCK_SIZE;
    return cache;
}

//add block to cache 
void add_to_cache(Cache* cache, char *hostname, char *path, char *data, int n) {
    printf("add to cache\n");
    CacheBlock* blocksptr;
    if (cache->nblocks < cache->maxblocks) { //add new block 
        printf("added new block\n");
        blocksptr = &cache->blocks[cache->nblocks];
        cache->nblocks++;
    } else { //cache is full
        printf("max blocks reached... replacing\n");
        int index = cache->maxblocks % n; // random replacement
        blocksptr = &cache->blocks[index];
    }
    // copy to block
    memcpy(blocksptr->data, data, n);
    blocksptr->size = n;
    printf("data size: %i\n", blocksptr->size);
    sprintf(blocksptr->host, hostname);
    sprintf(blocksptr->path, path);
    // printf("    block-data: %s, orig-data: %s\n", blocksptr->data, data);
    printf("    block-host: %s, orig-host: %s\n", blocksptr->host, hostname);
    printf("    block-path: %s, orig-path: %s\n", blocksptr->path, path);
}

//get block from cache 
CacheBlock * get_cached_data(Cache* cache, char *hostname, char *path) {
    printf("get cached data\n");
    printf("chached data matches, host = %s, path = %s\n", hostname, path);
    printf("[");
    for (int i=0; i<cache->nblocks; i++) {
        CacheBlock* blocksptr = &cache->blocks[i];
        printf("phost = %s, ppath = %s ", blocksptr->host, blocksptr->path);
        // check if path and tag match
        if (!strcmp(hostname, blocksptr->host) && !strcmp(path, blocksptr->path)) { //match found
            printf("found match");
            printf("]\n");
            return blocksptr;
        }
    }
    //match not found 
    printf("]\n");
    return NULL;

}

