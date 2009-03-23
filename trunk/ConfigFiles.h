/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 * Some inclusions, definitions, structures, varibles, and declarations for functions.
 */

#ifndef _Config_Files_H_
#define _COnfig_Files_H_

#define COLOR_FILE ".colorsrc" /* file to save picked HSV values */

#define H_OVERMEASURE 0
#define S_OVERMEASURE 0
#define V_OVERMEASURE 0

#include "Common.h"
#include "ColorIdentify.h"

struct HSVColors {
	struct HSVColor HSVColor[MAX_POINTS_PER_COLOR];
	int num;
	int name; /* just a pointer to the defined color */
}; /* a set of HSVColor-s of a single color */

struct ColorIdentifier {
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

int RecordColor(FILE *, struct HSVColors);
int ReadColor();

#endif
