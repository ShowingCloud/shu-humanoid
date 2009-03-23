/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>

#include "Decision.h"
#include "BottomLayer.h"
#include "Visiond.h"

int client_index[SOCKET_IDS], steps_start = 0, steps_middle, step_id = 0, num = 0, period = 0;
struct motor_step step_now, step_last, steps[200];

int MakeDecision ()
{
	FILE *fp;
	int i, tmp;

	fp = fopen ("pace/10-26/WalkStart", "r");
	while (!feof (fp)) {
		for (i = 0; i < 24; i++) {
			fscanf (fp, "%d,", &tmp);
			steps[steps_start].onestep[i] = tmp;
		}
		steps_start++;
	}
	steps_start--;
	fclose (fp);
	steps_middle = steps_start;
	fp = fopen ("pace/10-26/WalkMiddle", "r");
	while (!feof (fp)) {
		for (i = 0; i < 24; i++) {
			fscanf (fp, "%d,", &tmp);
			steps[steps_middle].onestep[i] = tmp;
		}
		steps_middle++;
	}
	steps_middle--;
	fclose (fp);

	return 1;
}

struct motor_step GetNextStep (struct VideoInfo video_info)
{
	struct motor_step ret;
#if 0
	if (step_id >= step_num) {
		if (step_status == STEP_INIT)
			ret = step_init;
		else if (step_status == STEP_WALK_START || step_status == STEP_WALK_PERIOD)
	ret = steps[step_id++];
	if (step_id >= step_num)
		step_status = STEP_INIT;
#endif
//	ret = step_now;
//	if (video_info.area[COLOR_YELLOW] > 0) {
//		if (video_info.aver_x[COLOR_YELLOW] > 360)
//			ret.onestep[20] += 2;
//		else if (video_info.aver_x[COLOR_YELLOW] < 280)
//			ret.onestep[20] -= 2;
//		if (video_info.aver_y[COLOR_YELLOW] > 270)
//			ret.onestep[21] += 2;
//		else if (video_info.aver_y[COLOR_YELLOW] < 210)
//			ret.onestep[21] -= 2;
//	}
	if (step_id >= steps_middle) step_id = steps_start;
	ret = steps[step_id++];
	return ret;
}

int main(int argc, char **argv)
{
	int listener_sockfd, client_sockfd, result, fd, nread;
	int client_id[FD_SETSIZE], max_fd = 2, i, j;
	socklen_t client_len;
	struct sockaddr_in client_address;
	fd_set readfds, testfds;
	struct VideoInfo video_info;
	struct timeval tv;

	step_now = step_last = step_init;
	memset (client_index, 0x00, SOCKET_IDS);
	memset ((void *) &video_info, 0x00, sizeof (struct VideoInfo));

	listener_sockfd = InitSocket (SOCKET_LISTENER_ID, "", 1);

	FD_ZERO (&readfds);
	FD_SET (listener_sockfd, &readfds);
	if (listener_sockfd > max_fd) max_fd = listener_sockfd;

	printf ("Socket listener waiting. A maximum of %d clients are allowed.\n", FD_SETSIZE);
	tv.tv_sec = 0;
	tv.tv_usec = 20000;
	client_len = sizeof (client_address);
	MakeDecision();

	while (1) {
		testfds = readfds;

		if ((result = select (FD_SETSIZE, &testfds, NULL, NULL, &tv)) < 0) {
			perror ("Decision");
			exit(-1);
		} else if (result == 0) {
			if (client_index[MOTORD_ID] != 0) {
				step_now = GetNextStep (video_info);
				write (client_index[MOTORD_ID], &step_now, sizeof (struct motor_step));
			}
			tv.tv_sec = 0;
			tv.tv_usec = 20000;
		} else {
			for (fd = 0; fd <= max_fd; fd++)
				if (FD_ISSET (fd, &testfds)) {
					if (fd == listener_sockfd) {
						if ((client_sockfd = accept (listener_sockfd,
										(struct sockaddr *) &client_address,
										&client_len)) < 0) {
							perror ("Decision");
							exit (-1);
						}

						FD_SET (client_sockfd, &readfds);
#ifdef VERBOSE
						printf ("Adding client on fd %d\n", client_sockfd);
#endif
						if (client_sockfd > max_fd) max_fd = client_sockfd;
	
						read (client_sockfd, &client_id[client_sockfd], sizeof (int));
						if (client_id[client_sockfd] <= 0 || client_id[client_sockfd] >= SOCKET_IDS) {
							printf ("Unknown client id %d. Closing.\n", client_id[client_sockfd]);
							close (client_sockfd);
							FD_CLR (client_sockfd, &readfds);
						} else
							client_index[client_id[client_sockfd]] = client_sockfd;
					} else {
						ioctl (fd, FIONREAD, &nread);

						if (nread == 0) {
							client_index[client_id[fd]] = 0;
							close (fd);
							FD_CLR (fd, &readfds);
						} else {
							if (client_id[fd] == VISIOND_ID) {
								read (fd, &video_info, sizeof (struct VideoInfo));
								if (client_index[MOTORD_ID] != 0)
//									 MakeDecision (video_info);
								if (client_index[CONSOLE_GUARDER_ID] != 0)
									write (client_index[CONSOLE_GUARDER_ID], &video_info, sizeof (struct VideoInfo));
							}
						}
					}
				}
		}
	}

	return -1; /* We never gets here. */
}
