#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "Paint.h"
#include "BottomLayer.h"

int PrintColor(unsigned char *frame, int c, int color)
{
	if (c >= 0 && c <= CAPTURE_WIDTH * CAPTURE_HEIGHT - 1)
	{
		frame[c * 3] = COLOR_VALUE_RGB[color][0];
		frame[c * 3 + 1] = COLOR_VALUE_RGB[color][1];
		frame[c * 3 + 2] = COLOR_VALUE_RGB[color][2];
	}
	return 1;
}

int DrawBigPoint(unsigned char *frame, int Center, int Radius, int color)
{
	int i, j;

	for (i = 0; i < Radius; i++)
		for (j = 0; j < Radius; j++)
		{
			PrintColor(frame, Center + i, color);
			PrintColor(frame, Center - i, color);
			PrintColor(frame, Center + CAPTURE_WIDTH * i + j, color);
			PrintColor(frame, Center + CAPTURE_WIDTH * i - j, color);
			PrintColor(frame, Center - CAPTURE_WIDTH * i + j, color);
			PrintColor(frame, Center - CAPTURE_WIDTH * i - j, color);
		}

	return 1;
}
