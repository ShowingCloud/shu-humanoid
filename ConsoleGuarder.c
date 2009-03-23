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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ConsoleGuarder.h"

int main(int argc, char** argv)
{
	int sockfd, result, i, sock_id = CONSOLE_GUARDER_ID, server_id;
	socklen_t len;
	struct sockaddr_in address;
	struct VideoInfo video_info;
	char info[500], append[300];

	sockfd = socket (AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr ("127.0.0.1");
	address.sin_port = htons (10200);
	len = sizeof (address);

	if ((result = connect (sockfd, (struct sockaddr *) &address, len)) == -1)
	{
		perror ("oops: ConsoleGuarder");
		exit(-1);
	}

	write (sockfd, &sock_id, sizeof (int));
	read (sockfd, &server_id, sizeof (int));
	if ((server_id & ID_MASK) != SOCKET_LISTENER_ID)
	{
		printf ("Error: unknown socket server!\n");
		exit(-1);
	}
	else
		printf ("Connected with the socket server!\n");

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
