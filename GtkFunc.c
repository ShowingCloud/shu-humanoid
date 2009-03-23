#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "GtkFunc.h"
#include "BottomLayer.h"
#include "SocketServer.h"
#include "Visiond.h"
#include "ColorIdentify.h"

int frame_id = 0, frame_inited = 0, do_searching = 1, gait_inited = 0, pagenum = 1;
unsigned char *frame_map;
int gait_sockfd, blocked_sockfd = 0;

gboolean socket_event(GIOChannel* iochannel, GIOCondition condition, gpointer data)
{
	char info[500], append[300];
	struct VideoInfo video_info;
	int i, client_id = GTK_GUARDER_ID;

	if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL)) {
		return 0;
	} else {
		read (g_io_channel_unix_get_fd (iochannel), &video_info, sizeof (struct VideoInfo));

		if (do_searching)
			client_id |= DO_SEARCHING;
		else
			client_id &= ~DO_SEARCHING;

		write (g_io_channel_unix_get_fd (iochannel), &client_id, sizeof (int));

		sprintf(info, "frames per second: %d\tseconds per frame: %1.3f", video_info.fps, video_info.spf);
		for (i = 0; i < COLOR_TYPES; i++) {
			sprintf(append, "\n%s\tarea: %d\n\taverage X: %d\taverage Y: %d",
					COLOR_NAME[i], video_info.area[i], video_info.aver_x[i], video_info.aver_y[i]);
			strcat(info, append);
		}
		gtk_label_set_text ((GtkLabel *) SearchResult, info);
		return 1;
	}
}

gboolean socket_frame_event(GIOChannel* iochannel, GIOCondition condition, gpointer data)
{
	GdkPixbuf *pixbuf;
	int client_id = GTK_GUARDER_FRAME_ID;
	struct vision_datagram vdgram;
	struct sockaddr_in address;
	socklen_t len;

	if (!frame_inited) {
		if (!(frame_map = (unsigned char *) malloc (CAPTURE_WIDTH * CAPTURE_HEIGHT * 3))) {
			perror ("malloc");
			exit(-1);
		}
		frame_inited = 1;
	}

	if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL)) {
		return 0;
	} else {
		recvfrom (g_io_channel_unix_get_fd (iochannel), &vdgram, sizeof (vdgram), 0,
				(struct sockaddr *) &address, &len);
		memcpy (vdgram.datagram, frame_map + vdgram.num * LARGEST_DATAGRAM, LARGEST_DATAGRAM);

		printf ("%d\n", vdgram.num);
		pixbuf = gdk_pixbuf_new_from_data ((unsigned char *) frame_map, GDK_COLORSPACE_RGB, FALSE,
				8, CAPTURE_WIDTH, CAPTURE_HEIGHT, CAPTURE_WIDTH * 3, NULL, NULL);

		if (pagenum == 0)
			gtk_image_set_from_pixbuf (GTK_IMAGE (image_info), pixbuf);
		else if (pagenum == 1)
			gtk_image_set_from_pixbuf (GTK_IMAGE (image_vision), pixbuf);

		return 1;
	}
}

gboolean StartStopSearching (GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
		do_searching = 1;
		return 1;
	} else {
		do_searching = 0;
		return 1;
	}
}

gboolean Adjusted (GtkAdjustment *adjustment, gpointer user_data)
{
	int i, onestep, server_id;
	char labeltxt[50];
	struct motor_step step;

	for (i = 0; i < MOTOR_NUM; i++)	{
		onestep = (int) gtk_adjustment_get_value (Adjustment[i]);
		sprintf (labeltxt, "Motor %d: %d", i, onestep);
		gtk_label_set_text ((GtkLabel *) ScrollLabel[i], labeltxt);
		step.onestep[i] = onestep;
	}

	if (!gait_inited) {
		gait_sockfd = InitSocket (GAIT_ADJUST_ID, "Gait Adjustment", &server_id, REMOTE_ADDR, SOCKET_TCP, 0);
		gait_inited = 1;
	}

//	while (gait_mutex);
//	gait_mutex = 1;
	write (gait_sockfd, &step, sizeof (struct motor_step));
	read (gait_sockfd, &server_id, sizeof (int));
//	gait_mutex = 0;

	return 1;
}

gboolean PageChanged (GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, gpointer user_data)
{
	pagenum = page_num;

	switch (page_num) {
		case 0:
			if (gait_inited) {
				close (gait_sockfd);
				gait_inited = 0;
			}
			if (blocked_sockfd) {
				int client_id = GTK_GUARDER_FRAME_ID;
				write (blocked_sockfd, &client_id, sizeof (int));
				blocked_sockfd = 0;
			}
			break;
		case 1:
			if (gait_inited) {
				close (gait_sockfd);
				gait_inited = 0;
			}
			if (blocked_sockfd) {
				int client_id = GTK_GUARDER_FRAME_ID;
				write (blocked_sockfd, &client_id, sizeof (int));
				blocked_sockfd = 0;
			}
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (SearchButton), FALSE);
			break;
		case 2:
			break;
		default:
			break;
	}

	return 1;
}

gboolean motion_notify (GtkWidget *event_box, GdkEventButton *event, gpointer callback_data)
{
	char text[500];
	int c = event->y * CAPTURE_WIDTH + event->x;
	struct HSVColor HSV;

	if (frame_map) {
		HSV = RGB2HSV (frame_map[c * 3], frame_map[c * 3 + 1], frame_map[c * 3 + 2]);
		sprintf (text, "Point Property:\nX: %3d\t\tY: %3d\nR: %3d\t\tG: %3d\t\tB: %3d\nH: %3d\t\tS: %3d\t\tV: %3d",
				(int) event->x, (int) event->y,
				frame_map[c * 3], frame_map[c * 3 + 1], frame_map[c * 3 + 2],
				HSV.H, HSV.S, HSV.V);
	} else
		sprintf (text, "Point Property:\nX: %3d\t\tY: %3d\nR: N/A\t\tG: N/A\t\tB: N/A\nH: N/A\t\tS: N/A\t\tV: N/A",
				(int) event->x, (int) event->y);

	gtk_label_set_text ((GtkLabel *) point_info, text);

	return 1;
}

gboolean button_press (GtkWidget *event_box, GdkEventButton *event, gpointer callback_data)
{
	char text[500];

	sprintf (text, "Point Property:\nX: %d\tY: %d\nR: %d\tG: %d\tB: %d\nH: %d\tS: %d\tV: %d",
			(int) event->x, (int) event->y, 0, 0, 0, 0, 0, 0);
	gtk_label_set_text ((GtkLabel *) point_info, text);

	return 1;
}
