/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 * Some inclusions, definitions, structures, varibles, and declarations for functions.
 */

#ifndef _Bottom_Layer_H_
#define _Bottom_Layer_H_

#define VIDEO_DEV "/dev/video0"
#define MOTORS_DEV "/dev/motors"

#define RM_EXEC_STEP 1
#define MOTOR_NUM 24

#define CAPTURE_WIDTH 640
#define CAPTURE_HEIGHT 480
#define CAPTURE_BPP 24
#define FRAME_PER_SECOND 10

#define LOCAL_ADDR "127.0.0.1"
#define REMOTE_ADDR "192.168.234.66"
//#define REMOTE_ADDR "192.168.234.69"

struct motor_step
{
	unsigned char onestep[MOTOR_NUM];
};

struct motor_motion
{
	short step_index;
	short motion_steps;
	struct motor_step *data;
};

struct motor_response
{
	unsigned short runmode;
	unsigned short retcode;
};

static const struct motor_step step_init = {{82,139,82,27,77,57,86,11,109,104,89,99,180,0,0,0,0,0,0,0,0,0,0,0}};

unsigned char *frame;

#include "Common.h"

int get_brightness_adj (unsigned char *, long, int *);
int InitVideo ();
int RetrieveFrame (int);
int CloseVideo (int);
void *InitShared (char *);
void *OpenShared (char *);
int CloseShared (void *);
int InitMotors ();
int SendMotors (int, struct motor_step);
struct motor_step ReadMotionFile (FILE *);
int InitSocket (int, char *, int *, char *);

#endif
