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
#define COLOR_TYPES 4
#define MAX_POINTS_PER_COLOR 20
#define SCATTER_INTERVAL_X 10
#define SCATTER_INTERVAL_Y 10
#define CONCENTRATED_INTERVAL_X 10
#define CONCENTRATED_INTERVAL_Y 10

#define H_OVERMEASURE 0
#define S_OVERMEASURE 0
#define V_OVERMEASURE 0

#define COLOR_RED 0
#define COLOR_GREEN 1
#define COLOR_BLUE 2
#define COLOR_YELLOW 3
#define COLOR_WHITE 4
#define COLOR_BLACK 5

#define CAPTURE_WIDTH 640
#define CAPTURE_HEIGHT 480
#define FRAME_PER_SECOND 10

/* names of the colors */
static const char color_name[][10] = {"red", "green", "blue", "yellow", "white", "black"};
/* standard color value, in RGB */
static const int color_value_rgb[][3] = {{0, 0, 255}, {0, 255, 0}, {255, 0, 0}, {0, 255, 255}, {255, 255, 255}, {0, 0, 0}};

int Index_Coordinate[CAPTURE_WIDTH * CAPTURE_HEIGHT], Index_Number[CAPTURE_WIDTH * CAPTURE_HEIGHT], Index_Length;

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
	int aver_S;
	int upper_limit_S;
	int lower_limit_S;
	int aver_V;
	int upper_limit_V;
	int lower_limit_V;
} identifier[COLOR_TYPES];

struct SearchResult
{
	int area;
	int aver_x;
	int aver_y;
} result[COLOR_TYPES];

struct PointMatched
{
	int capable;
	int color;
	int deviation_H;
	int deviation_S;
	int deviation_V;
};

struct FrameQueue
{
	int item[CAPTURE_WIDTH * CAPTURE_HEIGHT * 4];
	int head;
	int tail;
};

/* in My_Pickup.c */
void on_mouse(int, int, int, int, void*);

/* in My_Vision.c */
int SearchForColor(IplImage *, struct FrameQueue *, struct FrameQueue *);
struct PointMatched PointMatch(IplImage *, int, int);
int Scattering(IplImage *, struct FrameQueue *);
int SpreadPoints(IplImage *, struct FrameQueue *, struct FrameQueue *);
int Spreading(IplImage *, struct FrameQueue *, int);

/* in HSV.c */
struct HSVColor RGB2HSV(int, int, int);

/* in FrameQueue.c */
struct FrameQueue;
struct FrameQueue *InitQueue();
int Enqueue(struct FrameQueue *, int);
int Dequeue(struct FrameQueue *);
int ClearQueue(struct FrameQueue *);
int QueueLength(struct FrameQueue *);

/* in ColorFileOper.c */
int RecordColor(FILE *, struct HSVColors);
int ReadColor();

/* in GtkOper.c */
gboolean deleted(GtkWidget*, GdkEvent*, gpointer);

/* in Paint.c */
int PrintColor(IplImage *, int, int);
int DrawBigPoint(IplImage *, int, int, int);

#endif
