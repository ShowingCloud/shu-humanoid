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

struct Points points; /* record the coordinates of all points clicked */
struct HSVColors colors; /* record the HSV values of points clicked in one kind of color */
IplImage* frame; /* one frame */
FILE* fp; /* file to write HSV values in */


int RecordColor()
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

	fprintf(fp, "%d,%d,%s\n", colors.name, colors.num, color_name[colors.name]);

	for(i = 0; i < colors.num; i++)
	{
		fprintf(fp, "%d,%d,%d\n", colors.HSVColor[i].H, colors.HSVColor[i].S, colors.HSVColor[i].V);
	}

	return 1;
}


void on_mouse(int event, int x, int y, int flags, void* param)
{
	/*
	 * When left button is pressed down, record the color of this point,
	 * and convert it from RGB to HSV, then save it both in
	 * structure colors, which saves the HSV values of this kind of color,
	 * and structure points, which saves the HSV values of all points.
	 * At most 9 points are allowed to form one kind of color.
	 */
	if(event == CV_EVENT_LBUTTONDOWN && colors.num <= MAX_POINTS_PER_COLOR)
	{
		printf("%d, %d, %d, %d\n", event, x, y, flags); /* for debugging */

		points.point[points.num].x = x;
		points.point[points.num].y = y;
		points.point[points.num].color = COLOR_BLACK; /* display black by default */
		points.num++;

		int p = (y * CAPTURE_WIDTH + x) * 3; /* for each point, three integers (B, G, R) are saved */

		colors.HSVColor[colors.num] = RGB2HSV((unsigned char)frame->imageData[p + 2],
				(unsigned char)frame->imageData[p + 1], (unsigned char)frame->imageData[p]);
		printf("%d, %d, %d\n", colors.HSVColor[colors.num].H,
				colors.HSVColor[colors.num].S, colors.HSVColor[colors.num].V); /* for debugging */
		colors.num++;
	}
}

static gboolean deleted(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	return 1;
}

int main(int argc, char** argv)
{
	CvCapture *capture;
	GtkWidget *rate_dialog, *text;
	int i, j, p[9], frames = 0, frame_count = -1;
	struct timeval time_n, time_l, time_s;
	char info[80];

	/* capture from camera */
	if (!(capture = cvCreateCameraCapture(-1)))
	{
		fprintf(stderr, "Could not initialize capturing...\n");
		return -1;
	}

	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, CAPTURE_WIDTH);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, CAPTURE_HEIGHT);
//	cvSetCaptureProperty(capture, CV_CAP_PROP_FPS, FRAME_PER_SECOND);
	
	gtk_init(&argc, &argv);
	rate_dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(rate_dialog), "delete_event",	G_CALLBACK(deleted), NULL);
	text = gtk_label_new("frames per second: N/A\t\nseconds per frame: N/A");
	gtk_container_add(GTK_CONTAINER(rate_dialog), text);
	gtk_widget_show_all(rate_dialog);

	/* open a new window, and catch mouse events to it */
	cvNamedWindow("MyVision_PickingUp", 0);
	cvResizeWindow("MyVision_PickingUp", CAPTURE_WIDTH, CAPTURE_HEIGHT);
	cvSetMouseCallback("MyVision_PickingUp", on_mouse, 0);

	/* open the file for writing */
	if((fp = fopen(COLOR_FILE, "w")) == NULL)
	{
		fprintf(stderr, "Cannot open file...\n");
		return -1;
	}

	/* initial structure colors and points */
	colors.num = 0;
	points.num = 0;

	gettimeofday(&time_n, 0);
	time_s = time_l = time_n;

	for(;;)
	{
		/* query a frame */
		if (!(frame = cvQueryFrame(capture)))
			break;

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

		/*
		 * for points already recorded, mark them and their surrounding pixels
		 * (9 in all) with their color.
		 */
		for (i = 0; i < points.num; i++)
		{
			p[0] = (points.point[i].y * CAPTURE_WIDTH + points.point[i].x) * 3;
			p[1] = p[0] - 3; /* left pixel */
			p[2] = p[0] + 3; /* right pixel */
			for (j = 0; j < 3; j++)
			{
				p[3 + j] = p[j] - CAPTURE_WIDTH * 3; /* the line above */
				p[6 + j] = p[j] + CAPTURE_WIDTH * 3; /* the line below */
			}
			for (j = 0; j < 9; j++)
			{
				frame->imageData[p[j]] = color_value_rgb[points.point[i].color][0];
				frame->imageData[p[j] + 1] = color_value_rgb[points.point[i].color][1];
				frame->imageData[p[j] + 2] = color_value_rgb[points.point[i].color][2];
			}
		}

		/* show this modified frame */
		cvShowImage("MyVision_PickingUp", frame);

		switch((unsigned char)cvWaitKey(2))
		{
			case 'r': /* mark the former points as red, and save them */
				colors.name = COLOR_RED;
				while(!RecordColor());
				for (i = 0; i < colors.num; i++)
					points.point[points.num - i - 1].color = COLOR_RED;
				colors.num = 0;
				break;
			case 'g': /* mark the former points as green, and save them */
				colors.name = COLOR_GREEN;
				while(!RecordColor());
				for (i = 0; i < colors.num; i++)
					points.point[points.num - i - 1].color = COLOR_GREEN;
				colors.num = 0;
				break;
			case 'b': /* mark the former points as blue, and save them */
				colors.name = COLOR_BLUE;
				while(!RecordColor());
				for (i = 0; i < colors.num; i++)
					points.point[points.num - i - 1].color = COLOR_BLUE;
				colors.num = 0;
				break;
			case 'c': /* delete all points of the former color */
				points.num -= colors.num;
				colors.num = 0;
				break;
			case 'd': /* delete one former point of the former color, if there is */
				if (colors.num > 0)
				{
					points.num--;
					colors.num--;
				}
				break;
			case 27:
			case 'q': /* quit */
				goto quit;
			case ' ':
				cvWaitKey(0);
			default:
				break;
		}
	}

quit:	fclose(fp); /* close the file */
	cvReleaseCapture(&capture); /* release the capture from the camera */
	cvDestroyWindow("MyVision_PickingUp"); /* shutdown the window */
	gtk_widget_destroy(rate_dialog);

	return 0;
}
