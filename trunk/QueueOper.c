#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "QueueOper.h"

struct Queue *InitQueue (size_t size, int quantity)
{
	struct Queue *queue = (struct Queue *) malloc (sizeof (struct Queue));
	queue->type = size;
	queue->item = malloc (size * quantity);
	queue->head = 0;
	queue->tail = -1;
	return queue;
}

int Enqueue (struct Queue *queue, void *item)
{
	queue->tail++;
	*(queue->item + queue->tail * queue->type) = item;
	return 1;
}

void *Dequeue (struct Queue *queue)
{
	if (queue->head <= queue->tail)
		return *(queue->item + queue->head++ * queue->type);
	else
		return (void *)-1;
}

int ClearQueue (struct Queue *queue)
{
	queue->head = 0;
	queue->tail = -1;
	return 1;
}

int QueueLength (struct Queue *queue)
{
	return queue->tail - queue->head +  1;
}

int FreeQueue (struct Queue *queue)
{
	free (queue->item);
	free (queue);
	return 1;
}
