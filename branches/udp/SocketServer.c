/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>

#include "SocketServer.h"
#include "BottomLayer.h"
#include "Visiond.h"
#include "ConfigFiles.h"

int has_video_info = 0, shared_inited = 0, frame_id = 64, frame_ready = 0;
struct VideoInfo video_info;
int client_index[SOCKET_IDS][10], client_nums[SOCKET_IDS];
int listener_status = SOCKET_LISTENER_ID/* | DO_SEARCHING*/;
unsigned char *frame_map, *frame_pointer;
struct motor_step step_now, step_last;
int updown = 0, gait_adjust = 0;

__inline__ int ServeVisiond (int fd)
{
	int i;

#ifdef VERBOSE
	printf ("Serving Visiond. param: %d\n", listener_status);
#endif

	read (fd, &video_info, sizeof (struct VideoInfo));
	write (fd, &listener_status, sizeof (int));
	has_video_info = 1;

	for (i = 0; i < 10; i++)
		if (unlikely(client_index[CONSOLE_GUARDER_ID][i] != 0))
			write (client_index[CONSOLE_GUARDER_ID][i], &video_info, sizeof (struct VideoInfo));

	for (i = 0; i < 10; i++)
		if (unlikely(client_index[GTK_GUARDER_ID][i] != 0))
			write (client_index[GTK_GUARDER_ID][i], &video_info, sizeof (struct VideoInfo));

	if (unlikely(!shared_inited)) {
		frame_map = (unsigned char *) OpenShared ("/dev/shm/vision");
		frame_pointer = frame_map;
		shared_inited = 1;
	}

	if (unlikely(client_nums[GTK_GUARDER_FRAME_ID] != 0)) {
		if (unlikely(frame_id == 64)) {
			frame_ready = 0;
			listener_status |= NEED_FRAME;
			frame_id = 65;
		} else if (unlikely(frame_id == 65)) {
			listener_status &= ~NEED_FRAME;
			frame_id = 0;
		} else if (unlikely(frame_id == 0)) {
			frame_ready = 1;
			frame_pointer = frame_map;

			for (i = 0; i < 10; i++)
				if (unlikely(client_index[GTK_GUARDER_FRAME_ID][i] != 0))
					write (client_index[GTK_GUARDER_FRAME_ID][i], frame_pointer, LARGEST_DATAGRAM);
			frame_pointer += LARGEST_DATAGRAM;
			frame_id++;
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

__inline__ int ServeConsoleGuarder (int fd)
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

__inline__ int ServeGtkGuarder (int fd)
{
	int client_id;

#ifdef VERBOSE
	printf ("Serving Gtk Guarder.\n");
#endif

	read (fd, &client_id, sizeof (int));

	if (likely(client_id & DO_SEARCHING))
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

	if (unlikely(client_nums[GTK_GUARDER_ID] == 0)) {
		frame_id = 64;
		listener_status &= ~NEED_FRAME;
	}

	return 1;
}

__inline__ int ServeGtkGuarderFrame (int fd)
{
	int client_id;

#ifdef VERBOSE
	printf ("Serving Gtk Guarder (Frame).\n");
#endif

	read (fd, &client_id, sizeof (int));

	if (likely(frame_ready && frame_id != 64)) { /* FIXME parallel receiving problem */
		write (fd, frame_pointer, LARGEST_DATAGRAM);
		frame_pointer += LARGEST_DATAGRAM;
		frame_id++;
	}

	return 1;
}

int BreakGtkGuarderFrame (int fd)
{
#ifdef VERBOSE
	printf ("Stopping Gtk Guarder (Frame).\n");
#endif

	return 1;
}

__inline__ int ServeMotord (int fd)
{
	int client_id;

#ifdef VERBOSE
	printf ("Serving Motord.\n");
#endif

	if (!gait_adjust) {
		if (likely(step_now.onestep[0] < 110 && step_now.onestep[0] > 60))
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

__inline__ int ServeGaitAdjust (int fd)
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
	socklen_t listener_len, client_len;
	struct sockaddr_in listener_address, client_address;
	fd_set readfds, testfds;

	connected[0] = connected[1] = connected[2] = connected[3] = 1;
	for (i = 4; i < FD_SETSIZE; i++)
		connected[i] = 0;

	for (i = 0; i < SOCKET_IDS; i++) {
		for (j = 0; j < 10; j++)
			client_index[i][j] = 0;
		client_nums[i] = 0;
	}

	step_last = ReadInitStep ();
	if (unlikely(step_last.onestep[0] == 0 && step_last.onestep[1] == 0)) {
		printf ("Didn't get initial step from config file. Using default.");
		step_now = step_last = step_init;
	} else
		step_now = step_last;

	listener_sockfd = socket (AF_INET, SOCK_STREAM, 0);
	listener_address.sin_family = AF_INET;
	listener_address.sin_addr.s_addr = htonl (INADDR_ANY);
	listener_address.sin_port = htons (10200);
	listener_len = sizeof (listener_address);
	bind (listener_sockfd, (struct sockaddr *) &listener_address, listener_len);
	listen(listener_sockfd, 5);
	FD_ZERO (&readfds);
	FD_SET (listener_sockfd, &readfds);

	printf ("Socket listener waiting. A maximum of %d clients are allowed.\n", FD_SETSIZE);

	while (1) {
		int fd, nread;

		testfds = readfds;

		result = select (FD_SETSIZE, &testfds, (fd_set *) 0, (fd_set *) 0, (struct timeval *) 0);
		if (unlikely(result < 1)) {
			perror ("SocketServer");
			exit(-1);
		}

		for (fd = 0; fd <= max_fd; fd++)
			if (unlikely(FD_ISSET (fd, &testfds))) {
				if (unlikely(fd == listener_sockfd)) {
					client_len = sizeof (client_address);
					client_sockfd = accept (listener_sockfd, (struct sockaddr *) &client_address,
							&client_len);
					FD_SET (client_sockfd, &readfds);
#ifdef VERBOSE
					printf ("Adding client on fd %d\n", client_sockfd);
#endif
					if (likely(client_sockfd > max_fd)) max_fd = client_sockfd;
				} else {
					ioctl (fd, FIONREAD, &nread);

					if (unlikely(nread == 0)) {
						client_nums[client_id[fd]]--;
						for (i = 0; client_index[client_id[fd]][i] != fd; i++);
						client_index[client_id[fd]][i] = 0;
						ClientOper[client_id[fd]].Break (fd);
						connected[fd] = 0;
						if (unlikely(fd == max_fd)) {
							for (i = fd; connected[i] == 0; i--);
							max_fd = i;
						}
						close (fd);
						FD_CLR (fd, &readfds);
					} else {
						if (unlikely(connected[fd] != 1)) {
							client_id[fd] = SelectClient (fd);
							if (unlikely(client_id[fd] <= 0 || client_id[fd] >= SOCKET_IDS)) {
								printf ("Unknown client id %d. Closing.\n", client_id[fd]);
								close (fd);
								FD_CLR (fd, &readfds);
							}
							if (unlikely(client_nums[client_id[fd]] == MAX_CLIENTS[client_id[fd]])) {
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
