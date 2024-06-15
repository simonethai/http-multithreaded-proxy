#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "csapp.h"
#include "cache.h"
#include "sbuf.c"
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 104900
#define N_THREADS 4

sbuf_t *sbuf; 
pthread_rwlock_t lock;
Cache *cache;

void *thread(void *vargp);
void handler(int connfd);

int main(int argc, char *argv[])
{
    int listenfd; 
    int *connfdp;
    int connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    pthread_t tid;

    listenfd = open_listenfd(argv[1]);
    cache = create_cache(); // create cache

    sbuf_init(&sbuf, N_THREADS);
     printf("after init\n");
    for (int i = 0; i < N_THREADS; i++) /* Create worker threads */
        Pthread_create(&tid, NULL, thread, NULL);
     printf("after create\n");
    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        printf("before acccept\n");
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
        printf("connected\n");
        sbuf_insert(&sbuf, connfd); /* Insert connfd in buffer */
        printf("insert\n");

    }
}

//thread
void *thread(void *vargp) {
    printf("thread\n");
    Pthread_detach(pthread_self());
    while (1) {
        // printf("b4 remove sbuf\n");
        int connfd = sbuf_remove(&sbuf); //remove connfd from sbuf 
        printf("handle rq\n");
        handler(connfd);
        printf("After handle\n");
        Close(connfd);
        printf("connection closed\n");
    }
    printf("exit while loop, thread done");
}

    
//handle client's request 
void handler(int connfd) {
    rio_t rio_to_client; 
    char header_buf[MAXLINE];
    char request1[MAXLINE]; // old request 
    char request2[MAXLINE]; // new request 

    char host[MAXLINE];
    char path[MAXLINE];
    char headers[MAXLINE];
    char port[MAXLINE];
    printf("handle req func\n");
    // read and parse client request 
    rio_readinitb(&rio_to_client, connfd);
    printf("after read init\n");
    rio_readlineb(&rio_to_client, request1, MAXLINE); //read first line 
    // printf("buf:%s", request1); 

    // parse headers
    int number;
    if (rio_readlineb(&rio_to_client, header_buf, MAXLINE)<0) {
        printf("Error");
        return;
    }
    while ((number = rio_readlineb(&rio_to_client, header_buf, MAXLINE)) > 2) { 
        printf("header:%s", header_buf);
        if (!strncmp(header_buf, "Host: ", 6))
            strcpy(host, header_buf);
        else
            memcpy(headers, header_buf, sizeof(header_buf));
    }

    // parse request
    char method[10];
    char url[MAXLINE];
    char version[MAXLINE];

    //extract parts from request line 
    sscanf(request1, "%s %s %s", method, url, version);
    // check whether http versions are 1.1 or 1.0
    if (strcmp(version, "HTTP/1.1") && strcmp(version, "HTTP/1.0")) {
        return;
    }
    // check whether method is GET 
    if (strcmp(method, "GET")) {
        return;
    }
    // parse absolute or relative URL //todo
    if (strncmp(url, "http://", 7)) { //relative
        strcpy(path, url); //need to add http://
        strcpy(url, "http://");
        memcpy(url, host, strlen(host));
        memcpy(url, path, strlen(path));
    } else {//absolute  
        printf("url = %s\n", url);
        char *host2;
        char *port2; 
        char *path2;
        char *ptr;
        char *url_cpy = url;
        printf("url_cpy = %s\n", url_cpy);
        char *colon = ":";

        //host 
        if (strstr(url_cpy+7, colon) != NULL) { //has port 
            printf("inside loop\n");
            host2 = strtok(url_cpy+7, ":");
            //port 
            // printf("1\n");
            port2 = strtok(NULL, "/");
            // printf("2\n");
            memcpy(port, port2, strlen(port2));
            // printf("3\n");
        } else { //no port = set port to 80
            // printf("inside else\n");
            host2 = strtok(url_cpy + 7, "/");
            memcpy(port, "80", 2); 
        }
        memcpy(host, host2, strlen(host2));
        //path 
        path2 = strtok(NULL, " ");
        if (path2 == NULL) { //no path 
            memcpy(path, "/", 1);
        } else {
            memcpy(path, path2, strlen(path2));
        }
        

        printf("host = %s\n", host);
        printf("path = %s\n", path);
        printf("path2 = %s\n", path2);
        printf("port = %s\n", port);
        printf("sizeof path = %i\n", strlen(path));
    }

    //put request together 
    sprintf(request2, "GET %s HTTP/1.0\r\nHost: %s\r\n%s\r\n", path, host, headers);
    printf("new request: %s", request2);

    // check cache
    pthread_rwlock_wrlock(&lock);
    CacheBlock *blockptr = get_cached_data(cache, host, path);
    pthread_rwlock_unlock(&lock);

    if (blockptr != NULL) { //found match in cache 
        printf("found inside cache\n");
        // printf("data = %s\n ", blockptr->data);
        rio_writen(connfd, blockptr->data, blockptr->size);

    } else { //not in cache, get from server 
        printf("Not found in cache, get from server\n");
        char get_buf[MAXLINE];
        char new_block_buffer[MAX_OBJECT_SIZE];            
        int serverfd;
        rio_t rio_server;
        char host_url[MAXLINE];
        sprintf(host_url, "http://%s", host);

        size_t n; 
        size_t cache_data_size = 0;

        //create connection
        printf("host url: %s, port: %s\n", host_url, port);
        serverfd = open_clientfd(host, port);
        if (serverfd < 0) {
            printf("Error");
            return;
        }
    

        // connect and send request 
        rio_readinitb(&rio_server, serverfd);
        rio_writen(serverfd, request2, strlen(request2));
    
        // parse and write to client
        while ((n = rio_readnb(&rio_server, get_buf, MAXLINE)) != 0) {
            // sprintf(new_block_buffer + n, get_buf);
            memcpy(new_block_buffer+cache_data_size, get_buf, n);
            // printf("\nnew_bock_buffer: %s\n", new_block_buffer);
            //copy data to block
            cache_data_size += n;
            // sprintf(new_block_buffer + n, get_buf);
            rio_writen(connfd, get_buf, n);
        }
        //add to cache
        printf("cache_data_size: %d\n", cache_data_size);
        printf("\nnew_bock_buffer: %s\n", new_block_buffer);

        pthread_rwlock_wrlock(&lock);
        add_to_cache(cache, host, path, new_block_buffer,cache_data_size);
        pthread_rwlock_unlock(&lock);
      
    
        //close connection 
        close(serverfd);
    }
    return;
}





