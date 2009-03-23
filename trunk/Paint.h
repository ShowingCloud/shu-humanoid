/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 * Some inclusions, definitions, structures, varibles, and declarations for functions.
 */

#ifndef _Paint_H_
#define _Paint_H_

#include "Common.h"

static const int COLOR_VALUE_RGB[][3] = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}, {255, 255, 0}, {255, 255, 255}, {0, 0, 0}};

int PrintColor(unsigned char *, int, int);
int DrawBigPoint(unsigned char *, int, int, int);

#endif
