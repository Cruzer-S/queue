#ifndef QUEUE_H__
#define QUEUE_H__

#include <stddef.h>

typedef struct queue *Queue;

typedef struct queue_operation {
	Queue (*create)(size_t );

	int (*enqueue)(Queue , void *);
	void *(*dequeue)(Queue );

	size_t (*get_size)(Queue );

	void (*destroy)(Queue );
} *QueueOperation;

#endif
