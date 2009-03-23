/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 * Some inclusions, definitions, structures, varibles, and declarations for functions.
 */

#ifndef _Scatter_Spread_H_
#define _Scatter_Spread_H_

#define SCATTER_INTERVAL_X 30
#define SCATTER_INTERVAL_Y 30
#define CONCENTRATED_INTERVAL_X 10
#define CONCENTRATED_INTERVAL_Y 10
#define TRAVERSAL_INTERVAL_X 1
#define TRAVERSAL_INTERVAL_Y 1

#include "QueueOper.h"
#include "BottomLayer.h"
#include "ColorIdentify.h"

int Index_Coordinate[CAPTURE_WIDTH * CAPTURE_HEIGHT], Index_Number[CAPTURE_WIDTH * CAPTURE_HEIGHT], Index_Length;

struct SearchResult
{
	int area;
	int aver_x;
	int aver_y;
} result[COLOR_TYPES];

#include "Common.h"

int Scattering(struct Queue *);
int SpreadPoints(unsigned char *, struct Queue *, struct Queue *);
int Spreading(unsigned char *, struct Queue *, int);

#endif
