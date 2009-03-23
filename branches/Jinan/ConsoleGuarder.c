/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "ConsoleGuarder.h"
#include "BottomLayer.h"
#include "Decision.h"
#include "Visiond.h"

int main(int argc, char** argv)
{
	int sockfd, server_id, i;
	struct VideoInfo video_info;
	char info[500], append[300];

	sockfd = InitSocket (CONSOLE_GUARDER_ID, REMOTE_ADDR, 0);

	for (;;)
	{
		read (sockfd, &video_info, sizeof (struct VideoInfo));

		sprintf (info, "frames per second: %d\tseconds per frame: %f\n", video_info.fps, video_info.spf);
		for (i = 0; i < COLOR_TYPES; i++)
		{
			sprintf (append, "%s\tarea: %d\taverage X: %d\taverage Y: %d\n",
					COLOR_NAME[i], video_info.area[i], video_info.aver_x[i], video_info.aver_y[i]);
			strcat (info, append);
		}

		puts (info);
	}

	close (sockfd);

	return 0;
}
