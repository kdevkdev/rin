#ifndef QUEUE_H
#define QUEUE_H


#include<string.h>
#include<pthread.h>
#include<stdlib.h>

struct queueentry{
	void * p;
	struct queueentry * pred;
};
typedef struct queueentry qentry_t;
struct queue{
	qentry_t * head;
	qentry_t * tail;
	int count;

	pthread_mutex_t  *mutex;
};
typedef struct queue queue_t;

void delete_queue(queue_t * queue);
queue_t * create_queue(void);

void * queue_dequeue(queue_t * queue);
void queue_enqueue(queue_t * queue, void * p);
void * queue_peek(queue_t * queue);


#endif
