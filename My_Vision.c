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
struct SearchResult result[COLOR_TYPES];

int Index_Coordinate[CAPTURE_WIDTH * CAPTURE_HEIGHT], Index_Number[CAPTURE_WIDTH * CAPTURE_HEIGHT], Index_Length;

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
				color_name[i], aver_H, upper_limit_H, lower_limit_H,
				aver_S, upper_limit_S, lower_limit_S,
				aver_V, upper_limit_V, lower_limit_V);
	}

	return 1;
}

struct PointMatched PointMatch(IplImage *frame, int Coordinate, int color)
{
	int i;
	
	if (Coordinate < 0 || Coordinate >= CAPTURE_WIDTH * CAPTURE_HEIGHT
			|| (Index_Coordinate[Coordinate] <= Index_Length
				&& Index_Number[Index_Coordinate[Coordinate]] == Coordinate))
	{
		struct PointMatched ret = {0, COLOR_TYPES};
		return ret;
	}

	struct HSVColor HSV = RGB2HSV((unsigned char)frame->imageData[Coordinate * 3 + 2],
		(unsigned char)frame->imageData[Coordinate * 3 + 1],
		(unsigned char)frame->imageData[Coordinate * 3]);

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
					struct PointMatched ret = {1, i};
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
				struct PointMatched ret = {1, color};
				return ret;
			}
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

int SearchForColor(IplImage *frame, struct FrameQueue *ScatteringQueue, struct FrameQueue *SpreadingQueue)
{
	int i, c;

	Index_Length = 1;
	ClearQueue(ScatteringQueue);

	for (i = 0; i < COLOR_TYPES; i++)
		result[i].area = 0;

	Scattering(frame, ScatteringQueue);
	Spreading(frame, ScatteringQueue, SpreadingQueue);

	for (i = 0; i < COLOR_TYPES; i++)
		if (result[i].area != 0)
		{
			c = result[i].aver_y * CAPTURE_WIDTH + result[i].aver_x;
			PrintColor(frame, c, COLOR_TYPES + 1);
			PrintColor(frame, c + 1, COLOR_TYPES + 1);
			PrintColor(frame, c - 1, COLOR_TYPES + 1);
			PrintColor(frame, c + CAPTURE_WIDTH, COLOR_TYPES + 1);
			PrintColor(frame, c + CAPTURE_WIDTH + 1, COLOR_TYPES + 1);
			PrintColor(frame, c + CAPTURE_WIDTH - 1, COLOR_TYPES + 1);
			PrintColor(frame, c - CAPTURE_WIDTH, COLOR_TYPES + 1);
			PrintColor(frame, c - CAPTURE_WIDTH + 1, COLOR_TYPES + 1);
			PrintColor(frame, c - CAPTURE_WIDTH - 1, COLOR_TYPES + 1);
		}

	return 1;
}

int Scattering(IplImage *frame, struct FrameQueue *ScatteringQueue)
{
	int i, j;

	for (i = 0; i < CAPTURE_HEIGHT; i += SCATTER_INTERVAL_Y)
		for (j = 0; j < CAPTURE_WIDTH; j += SCATTER_INTERVAL_X)
			Enqueue(ScatteringQueue, (i * CAPTURE_WIDTH + j));

	return 1;
}


int Spreading(IplImage *frame, struct FrameQueue *ScatteringQueue, struct FrameQueue *SpreadingQueue)
{
	int NextPoint, x, y, aver_x, aver_y, area, specified_color;
	struct PointMatched Point_Matched;

	while(QueueLength(ScatteringQueue))
		if ((NextPoint = Dequeue(ScatteringQueue)) != -1)
		{
			Point_Matched = PointMatch(frame, NextPoint, -1);

			if (Point_Matched.capable)
			{
				specified_color = Point_Matched.color;
				ClearQueue(SpreadingQueue);
				area = 0; aver_x = 0; aver_y = 0;
				Enqueue(SpreadingQueue, NextPoint);
			
				while(QueueLength(SpreadingQueue))
					if ((NextPoint = Dequeue(SpreadingQueue)) != -1)
					{
						Point_Matched = PointMatch(frame, NextPoint, specified_color);

						if (Point_Matched.capable)
						{
							x = NextPoint % CAPTURE_WIDTH;
							y = NextPoint / CAPTURE_WIDTH;
							aver_x += x; aver_y += y;
							area++;

							Index_Coordinate[NextPoint] = Index_Length;
							Index_Number[Index_Length++] = NextPoint;
							PrintColor(frame, NextPoint, specified_color);

							if (NextPoint % CAPTURE_WIDTH != CAPTURE_WIDTH - 1)
								Enqueue(SpreadingQueue, NextPoint + 1);
							if (NextPoint % CAPTURE_WIDTH)
								Enqueue(SpreadingQueue, NextPoint - 1);
							Enqueue(SpreadingQueue, NextPoint + CAPTURE_WIDTH);
							Enqueue(SpreadingQueue, NextPoint - CAPTURE_WIDTH);
						}
					}

				if (area > result[specified_color].area)
				{
					aver_x /= area;
					aver_y /= area;

					result[specified_color].area = area;
					result[specified_color].aver_x = aver_x;
					result[specified_color].aver_y = aver_y;
				}
			}

		}

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
