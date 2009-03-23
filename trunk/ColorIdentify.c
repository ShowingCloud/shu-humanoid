#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "MyVision.h"

struct PointMatched PointMatch(unsigned char *frame, int Coordinate, int color)
{
	int i;
	
	if (Coordinate < 0 || Coordinate >= CAPTURE_WIDTH * CAPTURE_HEIGHT
			|| (Index_Coordinate[Coordinate] <= Index_Length && Index_Coordinate[Coordinate] != 0
				&& Index_Number[Index_Coordinate[Coordinate]] == Coordinate))
	{
		struct PointMatched ret = {0, COLOR_TYPES, -1, -1, -1};
		return ret;
	}

	struct HSVColor HSV = RGB2HSV(frame[Coordinate * 3], frame[Coordinate * 3 + 1], frame[Coordinate * 3 + 2]);

	if (color == -1)
		for (i = 0; i < COLOR_TYPES; i++)
		{
			if (identifier[i].aver_H != -500)
			{
				if ((HSV.H - identifier[i].aver_H) > 180) HSV.H -= 360;
				else if ((identifier[i].aver_H - HSV.H) > 180) HSV.H += 360;
				if (HSV.H >= identifier[i].lower_limit_H && HSV.H <= identifier[i].upper_limit_H
						&& HSV.S >= identifier[i].lower_limit_S
						&& HSV.S <= identifier[i].upper_limit_S
						&& HSV.V >= identifier[i].lower_limit_V
						&& HSV.V <= identifier[i].upper_limit_V)
				{
					struct PointMatched ret = {1, i, abs(HSV.H - identifier[i].aver_H) + 1,
						abs(HSV.S - identifier[i].aver_S) + 1,
						abs(HSV.V - identifier[i].aver_V) + 1};
					return ret;
				}
			}
		}
	else
	{
		if (identifier[color].aver_H != -500)
		{
			if ((HSV.H - identifier[color].aver_H) > 180) HSV.H -= 360;
			else if ((identifier[color].aver_H - HSV.H) > 180) HSV.H += 360;
			if (HSV.H >= identifier[color].lower_limit_H && HSV.H <= identifier[color].upper_limit_H
					&& HSV.S >= identifier[color].lower_limit_S
					&& HSV.S <= identifier[color].upper_limit_S
					&& HSV.V >= identifier[color].lower_limit_V
					&& HSV.V <= identifier[color].upper_limit_V)
			{
				struct PointMatched ret = {1, color, abs(HSV.H - identifier[color].aver_H) + 1,
					abs(HSV.S - identifier[color].aver_S) + 1,
					abs(HSV.V - identifier[color].aver_V) + 1};
				return ret;
			}
		}
	}

	struct PointMatched ret = {0, COLOR_TYPES, -1, -1, -1};
	return ret;
}
