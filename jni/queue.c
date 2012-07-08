#include "inc/queue.h"



queue_t * create_queue(void)
{
	queue_t * n = (queue_t *) malloc(sizeof(queue_t));
	n->head = NULL;
	n->tail  = NULL;
	n->count = 0;

	n->mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(n->mutex,NULL); 

	return n;
}
void delete_queue(queue_t * queue)
{
	//aint safe to use this, will cause unreferenced memory as values, mutexes and conditino_variables wont be delted
	pthread_mutex_lock(queue->mutex);

	pthread_mutex_t * tmp = queue->mutex;

	qentry_t * tail;
	while(queue->tail)
	{
		tail = queue->tail;
		queue->tail = queue->tail->pred;
		free(tail);
	}


	pthread_mutex_unlock(queue->mutex);
	pthread_mutex_destroy(tmp);
	
	free(queue->mutex);
	free(queue);
}
void * queue_dequeue(queue_t * queue)
{
	pthread_mutex_lock(queue->mutex);

	qentry_t * temp = queue->tail;
	void * p;
	void * res = NULL;

	if(temp)
	{
		if(queue->tail == queue->head)
		{
			queue->head = NULL;
			queue->tail = NULL;
		}
		else
		{
			queue->tail = queue->tail->pred;

		}
		p = temp->p;
		queue->count--;

		free(temp);
		res =  p;
	}
	pthread_mutex_unlock(queue->mutex);
	return res;
}
void * queue_peek(queue_t * queue)
{
	pthread_mutex_lock(queue->mutex);
	
	void * res = NULL;

	if(queue->tail)
	{
		res =  queue->tail->p;
	}
	pthread_mutex_unlock( queue->mutex );
	return res;
}
void queue_enqueue(queue_t * queue, void * p)
{
	pthread_mutex_lock(queue->mutex );

	qentry_t * n = (qentry_t *) malloc(sizeof(qentry_t));
	n->pred = NULL;
	n->p = p;
	
	if(queue->head)
		queue->head->pred = n;
	else
		queue->tail = n;

	queue->head = n;
	queue->count++;

	pthread_mutex_unlock(queue->mutex);
}
