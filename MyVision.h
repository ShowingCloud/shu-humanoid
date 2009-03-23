/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 * Some inclusions, definitions, structures, varibles, and declarations for functions.
 * Only for My_Pickup.c right now.
 */

#ifndef _My_Vision_H_
#define _My_Vision_H_

#include <gtk/gtk.h>
#include <cxcore.h>

#define COLOR_FILE "colordatafile.txt" /* file to save picked HSV values */
#define COLOR_TYPES 3
#define MAX_POINTS_PER_COLOR 20
#define SCATTER_INTERVAL_X 1
#define SCATTER_INTERVAL_Y 1

#define COLOR_RED 0
#define COLOR_GREEN 1
#define COLOR_BLUE 2
#define COLOR_WHITE 3
#define COLOR_BLACK 4

#define CAPTURE_WIDTH 640
#define CAPTURE_HEIGHT 480
#define FRAME_PER_SECOND 10

extern const char color_name[COLOR_TYPES + 2][10];
extern const int color_value_rgb[COLOR_TYPES + 2][3];

struct HSVColor
{
	int H;
	int S;
	int V;
}; /* a set of HSV values, of one point */

struct HSVColors
{
	struct HSVColor HSVColor[MAX_POINTS_PER_COLOR];
	int num;
	int name; /* just a pointer to the defined color */
}; /* a set of HSVColor-s of a single color */

struct Point
{
	int x;
	int y;
	int color; /* just a pointer to the defined color */
}; /* one point */

struct Points
{
	struct Point point[50];
	int num;
}; /* a set of point-s */

struct ColorIdentifier
{
	int aver_H;
	int upper_limit_H;
	int lower_limit_H;
};

struct PointMatched
{
	int capable;
	int color;
};

struct FrameQueue
{
	int item[CAPTURE_WIDTH * CAPTURE_HEIGHT];
	int head;
	int tail;
};

/* in My_Pickup.c */
int RecordColor();
void on_mouse(int, int, int, int, void*);
static gboolean deleted(GtkWidget*, GdkEvent*, gpointer);

/* in My_Vision.c */
int ReadColor();
int SearchForColor(IplImage *);
struct PointMatched PointMatch(struct HSVColor);
int PrintColor(IplImage *, int, int);

/* in HSV.c */
struct HSVColor RGB2HSV(int, int, int);

/* in FrameQueu */
struct FrameQueue;
struct FrameQueue *InitQueue();
int Enqueue(struct FrameQueue *, int);
int Dequeue(struct FrameQueue *);
int ClearQueue(struct FrameQueue *);
int QueueLength(struct FrameQueue *);

#endif
