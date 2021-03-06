/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 * Some inclusions, definitions, structures, varibles, and declarations for functions.
 */

#ifndef _Socket_Server_H_
#define _Socket_Server_H_

#define SOCKET_IDS 8
#define ID_MASK 0x000F

#define SOCKET_LISTENER_ID 0x0000
#define VISIOND_ID 0x0001
#define CONSOLE_GUARDER_ID 0x0002
#define GTK_GUARDER_ID 0x0003
#define GTK_GUARDER_FRAME_ID 0x0004
#define MOTORD_ID 0x0005
#define GAIT_ADJUST_ID 0x0006
#define MOTORD_HEAD_ID 0x0007

#define NEED_FRAME 0x0010
#define DO_SEARCHING 0x0020

#define DATAGRAM_ID_MASK 0x3F00

static const char SOCKET_ID[][20] = {
	"Socket Listener",
	"Visiond",
	"Console Guarder",
	"Gtk Guarder",
	"Gtk Guarder (Frame)",
	"Motord",
	"Gait Adjust"};
static const int MAX_CLIENTS[] = {1, 1, 10, 10, 10, 1, 1};

#define STEP_INIT 0
#define STEP_WALK_START 1
#define STEP_WALK_PERIOD 2
#define STEP_WALK_END 3
#define STEP_SIDLE 4
#define STEP_MOVE 5

#include "Common.h"

int InitClient ();

#endif
