/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 * Some inclusions, definitions, structures, varibles, and declarations for functions.
 * Only for My_Pickup.c right now.
 */

#ifndef _Bottom_Layer_H_
#define _Bottom_Layer_H_

#define VIDEO_DEV "/dev/video0"
#define MOTORS_DEV "/dev/motors"

#define RM_EXEC_STEP 1

#include "Common.h"

#if 0
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
#endif

//static const struct motor_step step_init = {{82,139,82,27,77,57,86,11,109,104,89,99,180,0,0,0,0,0,0,0,0,0,0,0}};

#endif
