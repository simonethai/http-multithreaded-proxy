/* $begin sbufc */
#include "csapp.h"
#include "sbuf.h"

/* Create an empty, bounded, shared FIFO buffer with n slots */
/* $begin sbuf_init */
void sbuf_init(sbuf_t *sp, int n)
{
    sp->buf = Calloc(n, sizeof(int)); 
    sp->n = n;                       /* Buffer holds max of n items */
    sp->front = sp->rear = 0;        /* Empty buffer iff front == rear */
    

//     if ((mutex = sem_open("/mutex", O_CREAT, 0644, 1)) == SEM_FAILED) {
//      perror("sem_open");
//      exit(EXIT_FAILURE);
//    }
    printf("before mutex init\n");
    sp->mutex = sem_open("/mutex", O_CREAT, 0644, 1);
    sp->slots = sem_open("/slots", O_CREAT, 0644, n);
    sp->items = sem_open("/items", O_CREAT, 0644, 0);
    printf("after mutex init\n");
    int sem_val;
    sem_getvalue(sp->items, &sem_val);
    printf("items: %d", sem_val);
    sem_getvalue(sp->slots, &sem_val);
    printf("slots: %d", sem_val);
    sem_getvalue(sp->mutex, &sem_val);
    printf("mutex: %d", sem_val);

    // Sem_init(&sp->mutex, 0, 1);      /* Binary semaphore for locking */
    // Sem_init(&sp->slots, 0, n);      /* Initially, buf has n empty slots */
    // Sem_init(&sp->items, 0, 0);      /* Initially, buf has zero data items */
}
/* $end sbuf_init */

/* Clean up buffer sp */
/* $begin sbuf_deinit */
void sbuf_deinit(sbuf_t *sp)
{
    Free(sp->buf);
}
/* $end sbuf_deinit */

/* Insert item onto the rear of shared buffer sp */
/* $begin sbuf_insert */
void sbuf_insert(sbuf_t *sp, int item)
{
    sem_wait(sp->slots);                          /* Wait for available slot */
    sem_wait(sp->mutex);                          /* Lock the buffer */
    printf("inserted %d sbuf\n", item);
    sp->buf[(++sp->rear)%(sp->n)] = item;   /* Insert the item */
    sem_post(sp->mutex);                          /* Unlock the buffer */
    sem_post(sp->items);                          /* Announce available item */
}
/* $end sbuf_insert */

/* Remove and return the first item from buffer sp */
/* $begin sbuf_remove */
int sbuf_remove(sbuf_t *sp)
{   


    int item;
    sem_wait(sp->items);                          /* Wait for available item */
    int sem_val;
    sem_getvalue(sp->items, sem_val);
    printf("items: %d", sem_val);
    sem_wait(sp->mutex);                          /* Lock the buffer */
    
    
    item = sp->buf[(++sp->front)%(sp->n)];  /* Remove the item */
    printf("removed %d sbuf\n", item);
    sem_post(sp->mutex);                          /* Unlock the buffer */
    sem_post(sp->slots);                          /* Announce available slot */
    return item;
}
/* $end sbuf_remove */
/* $end sbufc */

