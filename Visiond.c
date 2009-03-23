/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 * Picking up the colors from a window displaying the frame from camera by clicking. Click on 
 * the window, and that point will become black, and its HSV values are recorded. Then press one
 * key to indicate which color the points you clicked is to be, and their HSV values will be
 * saved to a file called "colordatafile.txt", which can be read by the scaning program.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/types.h>
#include <linux/videodev.h>

#include "Visiond.h"

int SearchForColor(unsigned char *frame, struct FrameQueue *ScatteringQueue, struct FrameQueue *SpreadingQueue)
{
	Scattering(ScatteringQueue);
	SpreadPoints(frame, ScatteringQueue, SpreadingQueue);

	return 1;
}

int main(int argc, char **argv)
{
	int video, offset;
	struct FrameQueue *ScatteringQueue, *SpreadingQueue;
	int frames = 0, frame_count = -1, i;
	struct timeval time_n, time_l, time_s;
	struct VideoInfo video_info;
	unsigned char *frame_map;

	int sockfd, sockresult, sock_id = VISIOND_ID, server_id;
	socklen_t len;
	struct sockaddr_in address;

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

	sockfd = socket (AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr ("127.0.0.1");
	address.sin_port = htons(10200);
	len = sizeof (address);

	if ((sockresult = connect (sockfd, (struct sockaddr *) &address, len)) == -1)
	{
		perror ("oops: Visiond");
		exit(-1);
	}

	write (sockfd, &sock_id, sizeof (int));
	read (sockfd, &server_id, sizeof (int));
	if ((server_id & ID_MASK) != SOCKET_LISTENER_ID)
	{
		printf ("Error: unknown socket server!\n");
		exit(-1);
	}
	else
		printf ("Connected with the socket server!\n");

	frame_map = (unsigned char *) InitShared ("/dev/shm/vision");

	ScatteringQueue = InitQueue ();
	SpreadingQueue = InitQueue ();

	gettimeofday (&time_n, 0);
	time_s = time_l = time_n;

	for(;;)
	{
		while ((offset = RetrieveFrame (video)) == -1);
		if (server_id & DO_SEARCHING)
			SearchForColor(frame + offset, ScatteringQueue, SpreadingQueue);
		if (server_id & NEED_FRAME)
			memcpy (frame_map, frame + offset, CAPTURE_WIDTH * CAPTURE_HEIGHT * 3);

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
		for (i = 0; i < COLOR_TYPES; i++)
		{
			video_info.area[i] = result[i].area;
			video_info.aver_x[i] = result[i].aver_x;
			video_info.aver_y[i] = result[i].aver_y;
		}

		time_l = time_n;

		write (sockfd, &video_info, sizeof (struct VideoInfo));
		read (sockfd, &server_id, sizeof (int));
	}

	free (ScatteringQueue);
	free (SpreadingQueue);
	close (sockfd);

	return 0;
}
