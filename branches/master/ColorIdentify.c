#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "ColorIdentify.h"
#include "BottomLayer.h"
#include "ScatterSpread.h"
#include "ConfigFiles.h"

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

struct HSVColor RGB2HSV (int R, int G, int B)
{
	/*
	 * Convert RGB color to HSV.
	 * TODO: This algorithm may or may not be efficient, which should be
	 * verified in thoroughly examination.
	 */
	int H, S, V;

	if (R > G)
	{
		if (B > R) /* max = B; min = G; B != 0; B != R; */
		{
			V = B;
			S = (B - G) * 255 / B;
			H = 240 + (R - G) * 60 / (B - G);
			struct HSVColor ret = {H, S, V};
			return ret;
		}
		else if (G < B) /* max = R; min = G; R != 0; R != G; */
		{
			V = R;
			S = (R - G) * 255 / R;
			H = 360 + (G - B) * 60 / (R - G);
			struct HSVColor ret = {H, S, V};
			return ret;
		}
		else /* B <= R && B <= G */ /* max = R; min = B; R != 0; R != B; */
		{
			V = R;
			S = (R - B) * 255 / R;
			H = (G - B) * 60 / (R - B);
			struct HSVColor ret = {H, S, V};
			return ret;
		}
	}
	else /* R <= G */
	{
		if (B < R) /* max = G; min = B; G != 0; G != B; */
		{
			V = G;
			S = (G - B) * 255 / G;
			H = 120 + (B - R) * 60 / (G - B);
			struct HSVColor ret = {H, S, V};
			return ret;
		}
		else if (G < B) /* max = B; min = R; B != 0; B != R; */
		{
			V = B;
			S = (B - R) * 255 / B;
			H = 240 + (R - G) * 60 / (B - R);
			struct HSVColor ret = {H, S, V};
			return ret;
		}
		else if (G != R) /* max = G; min = R; G != 0; G != R; */
		{
			V = G;
			S = (G - R) * 255 / G;
			H = 120 + (B - R) * 60 / (G - R);
			struct HSVColor ret = {H, S, V};
			return ret;
		}
		else /* B = R = G = B */ /* max = G; min = R; G == 0; G == R; */
		{
			V = G;
			S = 0;
			H = 0;
			struct HSVColor ret = {H, S, V};
			return ret;
		}
	}
}
