/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 * Some inclusions, definitions, structures, varibles, and declarations for functions.
 */

#ifndef _Color_Identify_H_
#define _Color_Identify_H_

#include "Common.h"

struct PointMatched {
	int capable;
	int color;
	int deviation_H;
	int deviation_S;
	int deviation_V;
};

struct HSVColor {
	int H;
	int S;
	int V;
}; /* a set of HSV values, of one point */

struct HSVColor RGB2HSV(int, int, int);
struct PointMatched PointMatch(unsigned char *, int, int);

#endif
