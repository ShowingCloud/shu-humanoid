#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "MyVision.h"

int RecordColor(FILE *fp, struct HSVColors colors)
{
	/*
	 * Save the HSV values of one color to the file.
	 * The file will look like this:
	 * 0,red
	 * xxx,xxx,xxx
	 * xxx,xxx,xxx
	 * 2,blue
	 * xxx,xxx,xxx
	 * ...
	 */
	int i;

	fprintf(fp, "%d,%d,%s\n", colors.name, colors.num, COLOR_NAME[colors.name]);

	for(i = 0; i < colors.num; i++)
	{
		fprintf(fp, "%d,%d,%d\n", colors.HSVColor[i].H, colors.HSVColor[i].S, colors.HSVColor[i].V);
	}

	return 1;
}

int ReadColor()
{
	/*
	 * Save the HSV values of one color to the file.
	 * The file will look like this:
	 * 1,red
	 * xxx,xxx,xxx
	 * xxx,xxx,xxx
	 * 3,blue
	 * xxx,xxx,xxx
	 * ...
	 */
	FILE *fp;
	int i, j, name, num;
	int aver_H_pre, aver_H, upper_limit_H, lower_limit_H;
	int aver_S, upper_limit_S, lower_limit_S;
	int aver_V, upper_limit_V, lower_limit_V;
	char colorname[10];
	struct HSVColors colors[COLOR_TYPES];

	if ((fp = fopen(COLOR_FILE, "r")) == NULL)
	{
		fprintf(stderr, "Cannot open file...\n");
		return -1;
	}

	for (i = 0; i < COLOR_TYPES; i++)
	{
		colors[i].name = i;
		colors[i].num = 0;
	}

	while (!feof(fp))
	{
		fscanf(fp, "%d,%d,%s\n", &name, &num, colorname);

		if ((colors[name].num + num) <= MAX_POINTS_PER_COLOR)
		{
			colors[name].num += num;
			for(i = colors[name].num - num; i < colors[name].num; i++)
				fscanf(fp, "%d,%d,%d\n", &colors[name].HSVColor[i].H,
						&colors[name].HSVColor[i].S, &colors[name].HSVColor[i].V);
		}
		else
			for(i = colors[name].num - num; i < colors[name].num; i++);
	}

	fclose(fp);
	
	for (i = 0; i < COLOR_TYPES; i++)
	{
		aver_H_pre = 0; aver_H = 0; upper_limit_H = 0; lower_limit_H = 360;
		aver_S = 0; upper_limit_S = 0; lower_limit_S = 255;
		aver_V = 0; upper_limit_V = 0; lower_limit_V = 255;
		if (colors[i].num != 0)
		{

			for (j = 0; j < colors[i].num; j++)
				aver_H_pre += colors[i].HSVColor[j].H;
			aver_H_pre /= colors[i].num;

			for (j = 0; j < colors[i].num; j++)
			{
				if ((colors[i].HSVColor[j].H - aver_H_pre) > 180) colors[i].HSVColor[j].H -= 360;
				else if ((aver_H_pre - colors[i].HSVColor[j].H) > 180) colors[i].HSVColor[j].H += 360;
				aver_H += colors[i].HSVColor[j].H;
				if (colors[i].HSVColor[j].H < lower_limit_H) lower_limit_H = colors[i].HSVColor[j].H;
				if (colors[i].HSVColor[j].H > upper_limit_H) upper_limit_H = colors[i].HSVColor[j].H;
				aver_S += colors[i].HSVColor[j].S;
				if (colors[i].HSVColor[j].S < lower_limit_S) lower_limit_S = colors[i].HSVColor[j].S;
				if (colors[i].HSVColor[j].S > upper_limit_S) upper_limit_S = colors[i].HSVColor[j].S;
				aver_V += colors[i].HSVColor[j].V;
				if (colors[i].HSVColor[j].V < lower_limit_V) lower_limit_V = colors[i].HSVColor[j].V;
				if (colors[i].HSVColor[j].V > upper_limit_V) upper_limit_V = colors[i].HSVColor[j].V;
			}
			aver_H /= colors[i].num;
			aver_S /= colors[i].num;
			aver_V /= colors[i].num;
		}
		else
		{
			aver_H = -500;
			aver_S = -500;
			aver_V = -500;
		}

		upper_limit_H += H_OVERMEASURE;
		lower_limit_H -= H_OVERMEASURE;
		if (upper_limit_S <= (255 - S_OVERMEASURE)) upper_limit_S += S_OVERMEASURE;
		else upper_limit_S = 255;
		if (lower_limit_S >= S_OVERMEASURE) lower_limit_S -= S_OVERMEASURE;
		else lower_limit_S = 0;
		if (upper_limit_V <= (255 - V_OVERMEASURE)) upper_limit_V += V_OVERMEASURE;
		else upper_limit_V = 255;
		if (lower_limit_V >= V_OVERMEASURE) lower_limit_V -= V_OVERMEASURE;
		else lower_limit_V = 0;
		

		identifier[i].aver_H = aver_H;
		identifier[i].upper_limit_H = upper_limit_H;
		identifier[i].lower_limit_H = lower_limit_H;
		identifier[i].aver_S = aver_S;
		identifier[i].upper_limit_S = upper_limit_S;
		identifier[i].lower_limit_S = lower_limit_S;
		identifier[i].aver_V = aver_V;
		identifier[i].upper_limit_V = upper_limit_V;
		identifier[i].lower_limit_V = lower_limit_V;

		printf("%s; H: %d, %d, %d; S: %d, %d, %d; V: %d, %d, %d\n",
				COLOR_NAME[i], aver_H, upper_limit_H, lower_limit_H,
				aver_S, upper_limit_S, lower_limit_S,
				aver_V, upper_limit_V, lower_limit_V);
	}

	return 1;
}

