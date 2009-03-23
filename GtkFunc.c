#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "GtkFunc.h"
#include "BottomLayer.h"
#include "SocketServer.h"
#include "Visiond.h"

int frame_id = 0, frame_inited = 0, do_searching = 1, gait_inited = 0;
unsigned char *frame_map, *frame_pointer;
int gait_sockfd, gait_mutex = 0;

gboolean socket_event(GIOChannel* iochannel, GIOCondition condition, gpointer data)
{
	char info[500], append[300];
	struct VideoInfo video_info;
	int i, client_id = GTK_GUARDER_ID;

	if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
	{
		return 0;
	}
	else
	{
		read (g_io_channel_unix_get_fd (iochannel), &video_info, sizeof (struct VideoInfo));

		if (do_searching)
			client_id |= DO_SEARCHING;
		else
			client_id &= ~DO_SEARCHING;

		write (g_io_channel_unix_get_fd (iochannel), &client_id, sizeof (int));

		sprintf(info, "frames per second: %d\tseconds per frame: %f", video_info.fps, video_info.spf);
		for (i = 0; i < COLOR_TYPES; i++)
		{
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

	if (!frame_inited)
	{
		if (!(frame_map = (unsigned char *) malloc (CAPTURE_WIDTH * CAPTURE_HEIGHT * 3)))
		{
			perror ("malloc");
			exit(-1);
		}
		frame_inited = 1;
		frame_pointer = frame_map;
	}

	if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
	{
		return 0;
	}
	else
	{
		if (frame_id == 64) /* a whole frame has been translated */
		{
			pixbuf = gdk_pixbuf_new_from_data ((unsigned char *) frame_map, GDK_COLORSPACE_RGB, FALSE,
					8, CAPTURE_WIDTH, CAPTURE_HEIGHT, CAPTURE_WIDTH * 3, NULL, NULL);
			gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);

			frame_id = 0;
			frame_pointer = frame_map;
		}

		read (g_io_channel_unix_get_fd (iochannel), frame_pointer, LARGEST_DATAGRAM);
		write (g_io_channel_unix_get_fd (iochannel), &client_id, sizeof (int));

		frame_pointer += LARGEST_DATAGRAM;
		frame_id++;

		return 1;
	}
}

gboolean StartStopSearching (GtkWidget *widget, gpointer data)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
	{
		do_searching = 1;
		return 1;
	}
	else
	{
		do_searching = 0;
		return 1;
	}
}

gboolean Adjusted (GtkAdjustment *adjustment, gpointer user_data)
{
	int i, onestep, server_id;
	char labeltxt[50];
	struct motor_step step;

	for (i = 0; i < MOTOR_NUM; i++)
	{
		onestep = (int) gtk_adjustment_get_value (Adjustment[i]);
		sprintf (labeltxt, "Motor %d: %d", i, onestep);
		gtk_label_set_text ((GtkLabel *) ScrollLabel[i], labeltxt);
		step.onestep[i] = onestep;
	}

	if (!gait_inited)
	{
		int result, sock_id = GAIT_ADJUST_ID;
		socklen_t len;
		struct sockaddr_in address;

		gait_sockfd = socket (AF_INET, SOCK_STREAM, 0);
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = inet_addr ("127.0.0.1");
		address.sin_port = htons (10200);
		len = sizeof (address);

		if ((result = connect (gait_sockfd, (struct sockaddr *) &address, len)) == -1)
		{
			perror ("oops: Gait Adjustment");
			exit(-1);
		}
	
		write (gait_sockfd, &sock_id, sizeof (int));
		read (gait_sockfd, &server_id, sizeof (int));
		if ((server_id & ID_MASK) != SOCKET_LISTENER_ID)
		{
			printf ("Error: unknown socket server!\n");
			exit(-1);
		}
		else
			printf ("Connected with the socket server! (gait adjust)\n");

		gait_inited = 1;
	}

	while (gait_mutex);
	gait_mutex = 1;
	write (gait_sockfd, &step, sizeof (struct motor_step));
	read (gait_sockfd, &server_id, sizeof (int));
	gait_mutex = 0;

	return 1;
}

gboolean PageChanged (GtkNotebook *notebook, GtkNotebookPage *page, guint page_num, gpointer user_data)
{
	switch (page_num)
	{
		case 1:
			if (gait_inited)
			{
				close (gait_sockfd);
				gait_inited = 0;
			}
			break;
		case 2:
			if (gait_inited)
			{
				close (gait_sockfd);
				gait_inited = 0;
			}
			break;
		case 3:
			break;
		default:
			break;
	}

	return 1;
}
