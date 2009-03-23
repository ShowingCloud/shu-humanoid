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

#include "SocketServer.h"
#include "BottomLayer.h"
#include "Visiond.h"

int has_video_info = 0, shared_inited = 0;
struct VideoInfo video_info;
int client_index[SOCKET_IDS][10], client_nums[SOCKET_IDS];
int listener_status = SOCKET_LISTENER_ID | DO_SEARCHING;
unsigned char *frame_map;
struct motor_step step_now, step_last;
int updown = 0, gait_adjust = 0, frame_sockfd;

int ServeVisiond (int fd)
{
	int i;
	struct vision_datagram vdgram;

#ifdef VERBOSE
	printf ("Serving Visiond. param: %d\n", listener_status);
#endif

	read (fd, &video_info, sizeof (struct VideoInfo));
	write (fd, &listener_status, sizeof (int));
	has_video_info = 1;

	for (i = 0; i < 10; i++)
		if (client_index[CONSOLE_GUARDER_ID][i] != 0)
			write (client_index[CONSOLE_GUARDER_ID][i], &video_info, sizeof (struct VideoInfo));

	for (i = 0; i < 10; i++)
		if (client_index[GTK_GUARDER_ID][i] != 0)
			write (client_index[GTK_GUARDER_ID][i], &video_info, sizeof (struct VideoInfo));

	if (!shared_inited) {
		frame_map = (unsigned char *) OpenShared ("/dev/shm/vision");
		shared_inited = 1;
	}

	if (client_nums[GTK_GUARDER_ID] != 0) {
		for (vdgram.num = 0; vdgram.num < DATAGRAM_NUM; vdgram.num++) {
			memcpy (frame_map + vdgram.num * LARGEST_DATAGRAM, vdgram.datagram, LARGEST_DATAGRAM);
			write (frame_sockfd, &vdgram, sizeof (struct vision_datagram));
//			sendto (frame_socket.sockfd, &vdgram, sizeof (struct vision_datagram), 0,
//					(struct sockaddr *) &frame_socket.address, frame_socket.len);
		}
	}

	return 1;
}

int BreakVisiond (int fd)
{
#ifdef VERBOSE
	printf ("Stopping Visiond.\n");
#endif

	has_video_info = 0;

	CloseShared ((void *) frame_map);
	shared_inited = 0;

	return 1;
}

int ServeConsoleGuarder (int fd)
{
#ifdef VERBOSE
	printf ("Serving Console Guarder.\n");
#endif

	return 1;
}

int BreakConsoleGuarder (int fd)
{
#ifdef VERBOSE
	printf ("Stopping Console Guarder.\n");
#endif

	return 1;
}

int ServeGtkGuarder (int fd)
{
	int client_id;

#ifdef VERBOSE
	printf ("Serving Gtk Guarder.\n");
#endif

	read (fd, &client_id, sizeof (int));

	if (client_id & DO_SEARCHING)
		listener_status |= DO_SEARCHING;
	else
		listener_status &= ~DO_SEARCHING;

	return 1;
}

int BreakGtkGuarder (int fd)
{
#ifdef VERBOSE
	printf ("Stopping Gtk Guarder.\n");
#endif

	if (client_nums[GTK_GUARDER_ID] == 0) {
//		frame_id = DATAGRAM_NUM;
//		listener_status &= ~NEED_FRAME;
	}

	return 1;
}

int ServeGtkGuarderFrame (int fd)
{
	int client_id;

#ifdef VERBOSE
	printf ("Serving Gtk Guarder (Frame).\n");
#endif

	read (fd, &client_id, sizeof (int));
#if 0
	if (frame_ready && frame_id != DATAGRAM_NUM) { /* FIXME parallel receiving problem */
		write (fd, frame_pointer, LARGEST_DATAGRAM);
		frame_pointer += LARGEST_DATAGRAM;
		frame_id++;
	}
#endif
	return 1;
}

int BreakGtkGuarderFrame (int fd)
{
#ifdef VERBOSE
	printf ("Stopping Gtk Guarder (Frame).\n");
#endif

	return 1;
}

int ServeMotord (int fd)
{
	int client_id;

#ifdef VERBOSE
	printf ("Serving Motord.\n");
#endif

	if (!gait_adjust) {
		if (step_now.onestep[0] < 110 && step_now.onestep[0] > 60)
			if (updown)
				step_now.onestep[0]++;
			else
				step_now.onestep[0]--;
		else {
			updown ^= 1;
	
			if (updown)
				step_now.onestep[0] = 61;
			else
				step_now.onestep[0] = 109;
		}
	}


	read (fd, &client_id, sizeof (int));
	write (fd, &step_now, sizeof (struct motor_step));

	step_last = step_now;

	return 1;
}

int BreakMotord (int fd)
{
#ifdef VERBOSE
	printf ("Stopping Motord.\n");
#endif

	return 1;
}

int ServeGaitAdjust (int fd)
{
#ifdef VERBOSE
	printf ("Serving Gait Adjust.\n");
#endif
	gait_adjust = 1;

	step_last = step_now;

	read (fd, &step_now, sizeof (struct motor_step));
	write (fd, &listener_status, sizeof (int));

	return 1;
}

int BreakGaitAdjust (int fd)
{
#ifdef VERBOSE
	printf ("Stopping Gait Adjust.\n");
#endif

	gait_adjust = 0;

	return 1;
}

static struct ClientOper ClientOper[] = {
	[VISIOND_ID] = {
		.Serve = ServeVisiond,
		.Break = BreakVisiond,
	},
	[CONSOLE_GUARDER_ID] = {
		.Serve = ServeConsoleGuarder,
		.Break = BreakConsoleGuarder,
	},
	[GTK_GUARDER_ID] = {
		.Serve = ServeGtkGuarder,
		.Break = BreakGtkGuarder,
	},
	[GTK_GUARDER_FRAME_ID] = {
		.Serve = ServeGtkGuarderFrame,
		.Break = BreakGtkGuarderFrame,
	},
	[MOTORD_ID] = {
		.Serve = ServeMotord,
		.Break = BreakMotord,
	},
	[GAIT_ADJUST_ID] = {
		.Serve = ServeGaitAdjust,
		.Break = BreakGaitAdjust,
	},
};

int main(int argc, char **argv)
{
	int listener_sockfd, client_sockfd, result;
	int connected[FD_SETSIZE], client_id[FD_SETSIZE], max_fd = 3, i, j;
	socklen_t client_len;
	struct sockaddr_in client_address;
	fd_set readfds, testfds;

	connected[0] = connected[1] = connected[2] = connected[3] = 1;
	for (i = 4; i < FD_SETSIZE; i++)
		connected[i] = 0;

	for (i = 0; i < SOCKET_IDS; i++) {
		for (j = 0; j < 10; j++)
			client_index[i][j] = 0;
		client_nums[i] = 0;
	}

	step_now = step_last = step_init;

	listener_sockfd = InitSocket (SOCKET_LISTENER_ID, "Socket Listener", (void *) 0, "", SOCKET_TCP, 1);
	frame_sockfd = InitSocket (GTK_GUARDER_FRAME_ID, "Gtk Guarder (Frame)", (void *) 0, "", SOCKET_UDP, 0);

	FD_ZERO (&readfds);

	FD_SET (listener_sockfd, &readfds);
	if (listener_sockfd > max_fd) max_fd = listener_sockfd;

//	FD_SET (listener_sockfd_udp, &readfds);
//	if (listener_sockfd_udp > max_fd) max_fd = listener_sockfd_udp;

	printf ("Socket listener waiting. A maximum of %d clients are allowed.\n", FD_SETSIZE);

	client_len = sizeof (client_address);

	while (1) {
		int fd, nread;

		testfds = readfds;

		result = select (FD_SETSIZE, &testfds, (fd_set *) 0, (fd_set *) 0, (struct timeval *) 0);
		if (result < 1)	{
			perror ("SocketServer");
			exit(-1);
		}

		for (fd = 0; fd <= max_fd; fd++)
			if (FD_ISSET (fd, &testfds)) {
				if (fd == listener_sockfd) {
					if ((client_sockfd = accept (listener_sockfd,
									(struct sockaddr *) &client_address,
									&client_len)) < 0) {
						perror ("SocketServer");
						exit (-1);
					}

					FD_SET (client_sockfd, &readfds);
#ifdef VERBOSE
					printf ("Adding client on fd %d\n", client_sockfd);
#endif
					if (client_sockfd > max_fd) max_fd = client_sockfd;

				} else if (fd == frame_sockfd) {
					printf ("Error\n");
					exit (-1);
				} else {
					ioctl (fd, FIONREAD, &nread);

					if (nread == 0) {
						client_nums[client_id[fd]]--;
						for (i = 0; client_index[client_id[fd]][i] != fd; i++);
						client_index[client_id[fd]][i] = 0;
						ClientOper[client_id[fd]].Break (fd);
						connected[fd] = 0;
						if (fd == max_fd) {
							for (i = fd; connected[i] == 0; i--);
							max_fd = i;
						}
						close (fd);
						FD_CLR (fd, &readfds);
					} else {
						if (connected[fd] != 1)	{
							client_id[fd] = SelectClient (fd);
							if (client_id[fd] <= 0 || client_id[fd] >= SOCKET_IDS) {
								printf ("Unknown client id %d. Closing.\n", client_id[fd]);
								close (fd);
								FD_CLR (fd, &readfds);
							}
							if (client_nums[client_id[fd]] == MAX_CLIENTS[client_id[fd]]) {
								printf ("Maximum allowed client %s numbers %d\
										has exceeded. Exiting.\n",
										SOCKET_ID[client_id[fd]],
										MAX_CLIENTS[client_id[fd]]);
								close (fd);
								FD_CLR (fd, &readfds);
							}
							connected[fd] = 1;

							client_nums[client_id[fd]]++;
							for (i = 0; client_index[client_id[fd]][i] != 0; i++);
							client_index[client_id[fd]][i] = fd;
						} else
							ClientOper[client_id[fd]].Serve (fd);
					}
				}
			}

	}

	return -1; /* We never gets here. */
}

int SelectClient (int fd)
{
	int client_id;

	read (fd, &client_id, sizeof (int));
	write (fd, &listener_status, sizeof (int));

	return client_id;
}
