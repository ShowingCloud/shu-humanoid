/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 * Some inclusions, definitions, structures, varibles, and declarations for functions.
 * Only for My_Pickup.c right now.
 */

#ifndef _Color_Identify_H_
#define _Color_Identify_H_

#include "Common.h"

struct ColorIdentifier
{
	int aver_H;
	int upper_limit_H;
	int lower_limit_H;
	int aver_S;
	int upper_limit_S;
	int lower_limit_S;
	int aver_V;
	int upper_limit_V;
	int lower_limit_V;
} identifier[COLOR_TYPES];

#endif
