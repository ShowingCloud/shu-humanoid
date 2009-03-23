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

struct ColorIdentifier identifier[COLOR_TYPES];

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
	int i, j, name, num, aver_H_pre, aver_H, upper_limit_H = 0, lower_limit_H = 360;
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
			}
			aver_H /= colors[i].num;
		}
		else
			aver_H = -500;

		identifier[i].aver_H = aver_H;
		identifier[i].upper_limit_H = upper_limit_H;
		identifier[i].lower_limit_H = lower_limit_H;

		printf("%s, %d, %d, %d\n", color_name[i], aver_H, upper_limit_H, lower_limit_H);
	}

	return 1;
}

struct PointMatched PointMatch(struct HSVColor HSV)
{
	int i;
	
	for (i = 0; i < COLOR_TYPES; i++)
	{
		if (identifier[i].aver_H != -500)
			if (HSV.H >= identifier[i].lower_limit_H && HSV.H <= identifier[i].upper_limit_H)
			{
				struct PointMatched ret = {1, i};
				return ret;
			}
	}
	struct PointMatched ret = {0, COLOR_TYPES};
	return ret;
}

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

int SearchForColor(IplImage *frame)
{
	int i, j, c, Length;
	struct PointMatched Point_Matched;
	struct FrameQueue *queue = InitQueue();
	
	for (i = 0; i < CAPTURE_HEIGHT; i += SCATTER_INTERVAL_Y)
		for (j = 0; j < CAPTURE_WIDTH; j += SCATTER_INTERVAL_X)
		{
			c = i * CAPTURE_WIDTH + j;
			Point_Matched = PointMatch(RGB2HSV((unsigned char)frame->imageData[c * 3 + 2],
							(unsigned char)frame->imageData[c * 3 + 1],
							(unsigned char)frame->imageData[c * 3]));
			if (Point_Matched.capable)
				Enqueue(queue, c);
		}

	Length = QueueLength(queue);
	for (i = 0; i < Length; i++)
		PrintColor(frame, Dequeue(queue), 3);

	return 1;
}

static gboolean deleted(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	return 1;
}

int main(int argc, char** argv)
{
	CvCapture *capture;
	IplImage *frame;
	GtkWidget *rate_dialog, *text;
	int frames = 0, frame_count = -1;
	struct timeval time_n, time_l, time_s;
	char info[80];

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
	cvSetCaptureProperty(capture, CV_CAP_PROP_FPS, FRAME_PER_SECOND);
	
	gtk_init(&argc, &argv);
	rate_dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(rate_dialog), "delete_event",	G_CALLBACK(deleted), NULL);
	text = gtk_label_new("frames per second: N/A\t\nseconds per frame: N/A");
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

		SearchForColor(frame);

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

quit:	cvReleaseCapture(&capture); /* release the capture from the camera */
	cvDestroyWindow("MyVision_Searching"); /* shutdown the window */
	gtk_widget_destroy(rate_dialog);

	return 0;
}
