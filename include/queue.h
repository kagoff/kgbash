#ifndef QUEUE_H
#define QUEUE_H

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

typedef struct Queue* queue_t;

queue_t queue_create    (void);
int     queue_destroy   (queue_t Q);
int     queue_enqueue   (queue_t Q, void* data);
int     queue_dequeue   (queue_t Q, void** return_data);
int     queue_count     (queue_t Q);
int     queue_test      (void);

#endif
