#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "MyVision.h"

struct FrameQueue *InitQueue()
{
	struct FrameQueue *queue = (struct FrameQueue *)malloc(sizeof(struct FrameQueue));
	queue->head = 0;
	queue->tail = -1;
	return queue;
}

int Enqueue(struct FrameQueue *queue, int item)
{
	queue->tail++;
	queue->item[queue->tail] = item;
	return 1;
}

int Dequeue(struct FrameQueue *queue)
{
	if (queue->head <= queue->tail)
		return queue->item[queue->head++];
	else
		return -1;
}

int ClearQueue(struct FrameQueue *queue)
{
	queue->head = 0;
	queue->tail = -1;
	return 1;
}

int QueueLength(struct FrameQueue *queue)
{
	return queue->tail - queue->head +  1;
}
