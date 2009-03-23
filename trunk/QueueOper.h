/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 * Some inclusions, definitions, structures, varibles, and declarations for functions.
 * Only for My_Pickup.c right now.
 */

#ifndef _Queue_Oper_H_
#define _Queue_Oper_H_

#include "Common.h"

struct Queue
{
	int type;
	void **item;
	int head;
	int tail;
};

struct Queue *InitQueue (size_t, int);
int Enqueue (struct Queue *, void *);
void *Dequeue (struct Queue *);
int ClearQueue (struct Queue *);
int QueueLength (struct Queue *);
int FreeQueue (struct Queue *);

#endif
