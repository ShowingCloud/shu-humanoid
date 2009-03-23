/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 * Some inclusions, definitions, structures, varibles, and declarations for functions.
 * Only for My_Pickup.c right now.
 */

#ifndef _Frame_Queue_H_
#define _Frame_Queue_H_

#include "Common.h"

struct FrameQueue
{
	int item[CAPTURE_WIDTH * CAPTURE_HEIGHT * 4];
	int head;
	int tail;
};

#endif
