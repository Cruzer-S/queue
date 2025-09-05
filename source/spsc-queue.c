#include "spsc-queue.h"

#include <stdatomic.h>
#include <stdlib.h>

struct queue {
	void **queue;
	size_t size;

	_Atomic(size_t) head;
	_Atomic(size_t) tail;
};

static Queue queue_create(size_t size)
{
	Queue q;

	q = malloc(sizeof(struct queue));
	if (q == NULL)
		return NULL;

	q->queue = malloc(sizeof(void *) * size);
	if (q->queue == NULL) {
		free(q);
		return NULL;
	}

	q->size = size;

	atomic_init(&q->head, 0);
	atomic_init(&q->tail, 0);

	return q;
}

static int queue_enqueue(Queue q, void *d)
{
	size_t h, t;

	h = atomic_load_explicit(&q->head, memory_order_acquire);
	t = atomic_load_explicit(&q->tail, memory_order_relaxed);
	if ((t + 1) % q->size == h)
		return -1;

	q->queue[t] = d;

	atomic_store_explicit(
		&q->tail, (t + 1) % q->size, memory_order_release
	);

	return (t + 1) % q->size;
}

static void *queue_dequeue(Queue q)
{
	size_t h, t;
	void *data;

 	h = atomic_load_explicit(&q->head, memory_order_relaxed);
	t = atomic_load_explicit(&q->tail, memory_order_acquire);
	if (h == t)
		return NULL;

	data = q->queue[h];

	atomic_store_explicit(
		&q->head, (h + 1) % q->size, memory_order_release
	);

	return data;
}

static size_t queue_get_size(Queue q)
{
	return q->size;
}

static void queue_destroy(Queue q)
{
	free(q->queue);
	free(q);
}

struct queue_operation spsc_queue_ops = {
	.create = queue_create,

	.enqueue = queue_enqueue,
	.dequeue = queue_dequeue,

	.get_size = queue_get_size,

	.destroy = queue_destroy
};
