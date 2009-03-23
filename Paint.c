#include "MyVision.h"

int PrintColor(IplImage *frame, int c, int color)
{
	if (c >= 0 && c <= CAPTURE_WIDTH * CAPTURE_HEIGHT - 1)
	{
		frame->imageData[c * 3 + 2] = color_value_rgb[color][2];
		frame->imageData[c * 3 + 1] = color_value_rgb[color][1];
		frame->imageData[c * 3] = color_value_rgb[color][0];
	}
	return 1;
}

int DrawBigPoint(IplImage *frame, int Center, int Radius, int color)
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
