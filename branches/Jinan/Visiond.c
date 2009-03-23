/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "Visiond.h"
#include "BottomLayer.h"
#include "QueueOper.h"
#include "Decision.h"
#include "ScatterSpread.h"
#include "ConfigFiles.h"

#define IMAGE_RAW "raw.ppm"
#define IMAGE_SEARCHED "searched.ppm"

int SearchForColor (unsigned char *frame, struct Queue *ScatteringQueue, struct Queue *SpreadingQueue)
{
	Scattering (ScatteringQueue);
	SpreadPoints (frame, ScatteringQueue, SpreadingQueue);

	return 1;
}

int main (int argc, char **argv)
{
	int video, offset;
	FILE *file;
	struct Queue *ScatteringQueue, *SpreadingQueue;
	int frames = 0, frame_count = -1, i;
	struct timeval time_n, time_l, time_s;
	struct VideoInfo video_info;
	int sockfd;

	if ((video = InitVideo ()) == -1)
	{
		perror ("oops: Visiond");
		exit(-1);
	}

	if (!ReadColor())
	{
		fprintf(stderr, "Error reading color data file...\n");
		return -1;
	}

	sockfd = InitSocket (VISIOND_ID, LOCAL_ADDR, 0);

	ScatteringQueue = InitQueue (sizeof (int), CAPTURE_WIDTH * CAPTURE_HEIGHT * 3);
	SpreadingQueue = InitQueue (sizeof (int), CAPTURE_WIDTH * CAPTURE_HEIGHT * 3);
	memset (Index_Coordinate, 0x00, 640 * 480);
	memset (Index_Number, 0x00, 640 * 480);

	gettimeofday (&time_n, 0);
	time_s = time_l = time_n;

	for(;;)
	{
		while ((offset = RetrieveFrame (video)) == -1)
			usleep (50000);
		SearchForColor(frame + offset, ScatteringQueue, SpreadingQueue);

		gettimeofday(&time_n, 0);
		frame_count++;
		if ((time_n.tv_sec - time_s.tv_sec) >= 1)
		{
			frames = frame_count;
			frame_count = -1;
			time_s = time_n;
		}

		video_info.fps = frames;
		video_info.spf = (time_n.tv_sec - time_l.tv_sec) + (float)(time_n.tv_usec - time_l.tv_usec) / 1000000;

		for (i = COLOR_TYPES - 1; i >= 0; i--)
		{
			video_info.area[i] = result[i].area;
			video_info.aver_x[i] = result[i].aver_x;
			video_info.aver_y[i] = result[i].aver_y;
		}

		time_l = time_n;

		write (sockfd, &video_info, sizeof (struct VideoInfo));

		usleep (500000);
	}

	FreeQueue (ScatteringQueue);
	FreeQueue (SpreadingQueue);
	close (sockfd);

	return 0;
}
