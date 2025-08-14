#include "spsc-queue.h"

#include <stdlib.h>
#include <stdio.h>
#include <threads.h>

typedef struct data {
	enum operation {
		OPS_ADD,
		OPS_SUB,
		OPS_STOP
	} ops;
	int value;
} *Data;

#define NTIMES 100
#define VALUE_RANGE 1024

int produce(void *q)
{
	Queue queue;
	int ret;

	queue = q;

	ret = 0;
	for (int i = 0; i < NTIMES; i++) {
		Data data = malloc(sizeof(struct data));
		if (data == NULL)
			return -1;

		if ((i + 1) == NTIMES)
			data->ops = OPS_STOP;
		else
			data->ops = rand() % OPS_STOP;

		data->value = rand() % VALUE_RANGE;
		switch (data->ops) {
		case OPS_ADD: ret += data->value; break;
		case OPS_SUB: ret -= data->value; break;
		default: break;
		}

		while (spsc_queue_ops->enqueue(queue, data) == -1)
			continue;
	}

	return ret;
}

int consumer(void *q)
{
	Queue queue;
	int ret;

	queue = q;

	ret = 0;
	for (int run; run; ) {
		Data data;

		while ((data = spsc_queue_ops->dequeue(queue)) == NULL)
			continue;

		switch (data->ops) {
		case OPS_ADD: ret += data->value; break;
		case OPS_SUB: ret -= data->value; break;
		case OPS_STOP: run = 0; break;
		}

		free(data);
	}

	return ret;
}

int main(void)
{
	Queue queue;
	thrd_t p, c;
	int res;

	queue = spsc_queue_ops->create(2);
	if (queue == NULL)
		exit(EXIT_FAILURE);

	srand((unsigned int) time(NULL));

	if (thrd_create(&p, produce, queue) != thrd_success)
		exit(EXIT_FAILURE);

	if (thrd_create(&c, consumer, queue) != thrd_success)
		exit(EXIT_FAILURE);

	if (thrd_join(p, &res) != thrd_success)
		exit(EXIT_FAILURE);

	printf("producer: %d\n", res);

	if (thrd_join(c, &res) != thrd_success)
		exit(EXIT_FAILURE);

	printf("consumer: %d\n", res);

	spsc_queue_ops->destroy(queue);

	return 0;
}
