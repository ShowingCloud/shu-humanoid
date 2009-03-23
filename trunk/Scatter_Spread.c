#include "MyVision.h"

int Scattering(IplImage *frame, struct FrameQueue *ScatteringQueue)
{
	int i, j;

	Index_Length = 1;
	ClearQueue(ScatteringQueue);

	for (i = 0; i < COLOR_TYPES; i++)
		result[i].area = 0;

	for (i = 0; i < CAPTURE_HEIGHT; i += SCATTER_INTERVAL_Y)
		for (j = 0; j < CAPTURE_WIDTH; j += SCATTER_INTERVAL_X)
			Enqueue(ScatteringQueue, (i * CAPTURE_WIDTH + j));

	return 1;
}


int SpreadPoints(IplImage *frame, struct FrameQueue *ScatteringQueue, struct FrameQueue *SpreadingQueue)
{
	int NextPoint, specified_color, i;
	struct PointMatched Point_Matched;

	while(QueueLength(ScatteringQueue))
		if ((NextPoint = Dequeue(ScatteringQueue)) != -1)
		{
			Point_Matched = PointMatch(frame, NextPoint, -1);

			if (Point_Matched.capable)
			{
				specified_color = Point_Matched.color;
				ClearQueue(SpreadingQueue);
				Enqueue(SpreadingQueue, NextPoint);
				Spreading(frame, SpreadingQueue, specified_color);
			}
		}

	for (i = 0; i < COLOR_TYPES; i++)
		if (result[i].area != 0)
			DrawBigPoint(frame, (result[i].aver_y * CAPTURE_WIDTH + result[i].aver_x), 3, COLOR_TYPES);

	return 1;
}

int Spreading(IplImage *frame, struct FrameQueue *SpreadingQueue, int specified_color)
{
	int NextPoint, x, y, aver_x = 0, aver_y = 0, area = 0, weight;
	struct PointMatched Point_Matched;

	while(QueueLength(SpreadingQueue))
		if ((NextPoint = Dequeue(SpreadingQueue)) != -1)
		{
			Point_Matched = PointMatch(frame, NextPoint, specified_color);

			if (Point_Matched.capable)
			{
				x = NextPoint % CAPTURE_WIDTH;
				y = NextPoint / CAPTURE_WIDTH;

				weight = 36 * 25 * 25 / Point_Matched.deviation_H
					/ Point_Matched.deviation_S / Point_Matched.deviation_V;
				aver_x += weight * x;
				aver_y += weight * y;
				area += weight;

				Index_Coordinate[NextPoint] = Index_Length;
				Index_Number[Index_Length++] = NextPoint;
				PrintColor(frame, NextPoint, specified_color);

				if (NextPoint % CAPTURE_WIDTH != CAPTURE_WIDTH - 1) Enqueue(SpreadingQueue, NextPoint + 1);
				if (NextPoint % CAPTURE_WIDTH) Enqueue(SpreadingQueue, NextPoint - 1);
				Enqueue(SpreadingQueue, NextPoint + CAPTURE_WIDTH);
				Enqueue(SpreadingQueue, NextPoint - CAPTURE_WIDTH);
			}
		}

	if (area > result[specified_color].area)
	{
		aver_x /= area;
		aver_y /= area;
//		area /= (36 * 25 * 25);

		result[specified_color].area = area;
		result[specified_color].aver_x = aver_x;
		result[specified_color].aver_y = aver_y;
	}

	return 1;
}

