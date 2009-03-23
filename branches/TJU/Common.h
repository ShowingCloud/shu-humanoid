/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 * Some inclusions, definitions, structures, varibles, and declarations for functions.
 */

#ifndef _Common_H_
#define _Common_H_

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x)  __builtin_expect(!!(x), 0)

#define MAX_POINTS_PER_COLOR 20

#define COLOR_TYPES 4

#define COLOR_RED 0
#define COLOR_GREEN 1
#define COLOR_BLUE 2
#define COLOR_YELLOW 3
#define COLOR_WHITE 4
#define COLOR_BLACK 5

static const char COLOR_NAME[][10] = {"red", "green", "blue", "yellow", "white", "black"};
//static const int COLOR_VALUE_RGB[][3] = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 0}, {255, 255, 255}, {0, 0, 0}};

#endif
