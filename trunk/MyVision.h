/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 * Some inclusions, definitions, structures, varibles, and declarations for functions.
 * Only for My_Pickup.c right now.
 */

#ifndef _My_Vision_H_
#define _My_Vision_H_
#include <cv.h>
#include <highgui.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define filename "colordatafile.txt" /* file to save picked HSV values */

#define COLOR_BLACK 0
#define COLOR_RED 1
#define COLOR_GREEN 2
#define COLOR_BLUE 3

const char color_name[][10] = {"black", "red", "green", "blue"}; /* names of the colors */
const int color_value_rgb[][3] = {{0, 0, 0}, {0, 0, 255}, {0, 255, 0}, {255, 0, 0}}; /* standard color value, in RGB */

struct HSVColor
{
	int H;
	int S;
	int V;
}; /* a set of HSV values, of one point */

struct HSVColors
{
	struct HSVColor HSVColor[10];
	int num;
	int name; /* just a pointer to the defined color */
}; /* a set of HSVColor-s of a single color */

struct Point
{
	int x;
	int y;
	int color; /* just a pointer to the defined color */
}; /* one point */

struct Points
{
	struct Point point[50];
	int num;
}; /* a set of point-s */

int RecordColor();
void on_mouse(int, int, int, int, void*);
struct HSVColor RGB2HSV(int, int, int);

#endif
