/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 * Some inclusions, definitions, structures, varibles, and declarations for functions.
 */

#ifndef _Visiond_H_
#define _Visiond_H_

#include "Common.h"
#include "QueueOper.h"

struct VideoInfo {
	int fps;
	float spf;
	int area[COLOR_TYPES];
	int aver_x[COLOR_TYPES];
	int aver_y[COLOR_TYPES];
};

int SearchForColor(unsigned char *, struct Queue *, struct Queue *);

#endif
