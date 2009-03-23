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
#include <cv.h>
#include <highgui.h>
#include <time.h>
#include <sys/time.h>
#include <gtk/gtk.h>

#include "MyVision.h"

int SearchForColor(IplImage *frame, struct FrameQueue *ScatteringQueue, struct FrameQueue *SpreadingQueue)
{
	Scattering(frame, ScatteringQueue);
	SpreadPoints(frame, ScatteringQueue, SpreadingQueue);

	return 1;
}

int main(int argc, char** argv)
{
	CvCapture *capture;
	IplImage *frame;
	GtkWidget *rate_dialog, *text;
	struct FrameQueue *ScatteringQueue = InitQueue(), *SpreadingQueue = InitQueue();
	int frames = 0, frame_count = -1, i;
	struct timeval time_n, time_l, time_s;
	char info[500], append[100];

	/* capture from camera */
	if (!(capture = cvCreateCameraCapture(-1)))
	{
		fprintf(stderr, "Could not initialize capturing...\n");
		return -1;
	}

	if (!ReadColor())
	{
		fprintf(stderr, "Error reading color data file...\n");
		return -1;
	}

	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, CAPTURE_WIDTH);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, CAPTURE_HEIGHT);
//	cvSetCaptureProperty(capture, CV_CAP_PROP_FPS, FRAME_PER_SECOND);
	
	gtk_init(&argc, &argv);
	rate_dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	g_signal_connect(G_OBJECT(rate_dialog), "delete_event",	G_CALLBACK(deleted), NULL);

	sprintf(info, "frames per second: N/A\t\nseconds per frame: N/A");
	for (i = 0; i < COLOR_TYPES; i++)
	{
		sprintf(append, "\n%s\n\tarea: N/A\n\taverage X: N/A\n\taverage Y: N/A", color_name[i]);
		strcat(info, append);
	}
	text = gtk_label_new(info);
	gtk_container_add(GTK_CONTAINER(rate_dialog), text);

	gtk_widget_show_all(rate_dialog);

	/* open a new window, and catch mouse events to it */
	cvNamedWindow("MyVision_Searching", 0);
	cvResizeWindow("MyVision_Searching", CAPTURE_WIDTH, CAPTURE_HEIGHT);

	gettimeofday(&time_n, 0);
	time_s = time_l = time_n;

	for(;;)
	{
		/* query a frame */
		if (!(frame = cvQueryFrame(capture)))
			break;

		SearchForColor(frame, ScatteringQueue, SpreadingQueue);

		gettimeofday(&time_n, 0);
		frame_count++;
		if ((time_n.tv_sec - time_s.tv_sec) >= 1)
		{
			frames = frame_count;
			frame_count = -1;
			time_s = time_n;
		}

		sprintf(info, "frames per second: %d\nseconds per frame: %f", frames,
				((time_n.tv_sec - time_l.tv_sec) + (float)(time_n.tv_usec - time_l.tv_usec) / 1000000));
		for (i = 0; i < COLOR_TYPES; i++)
		{
			sprintf(append, "\n%s\n\tarea: %d\n\taverage X: %d\n\taverage Y: %d",
					color_name[i], result[i].area, result[i].aver_x, result[i].aver_y);
			strcat(info, append);
		}
		gtk_label_set_text((GtkLabel *)text, info);

		time_l = time_n;

		/* show this modified frame */
		cvShowImage("MyVision_Searching", frame);

		switch((unsigned char)cvWaitKey(2))
		{
			case 27:
			case 'q': /* quit */
				goto quit;
			case ' ':
				cvWaitKey(0);
			default:
				break;
		}
	}

quit:	free(ScatteringQueue);
	free(SpreadingQueue);
	cvReleaseCapture(&capture); /* release the capture from the camera */
	cvDestroyWindow("MyVision_Searching"); /* shutdown the window */
	gtk_widget_destroy(rate_dialog);

	return 0;
}
