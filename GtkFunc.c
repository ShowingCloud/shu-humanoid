#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "GtkFunc.h"
#include "BottomLayer.h"
#include "SocketServer.h"
#include "Visiond.h"

int frame_id = 0, frame_inited = 0, do_searching = 1, gait_inited = 0, pagenum = 1;
unsigned char *frame_map, *frame_pointer;
int gait_sockfd, blocked_sockfd = 0;

gboolean socket_event(GIOChannel* iochannel, GIOCondition condition)
{
	char info[500], append[300];
	struct VideoInfo video_info;
	int i, client_id = GTK_GUARDER_ID;

	if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
		return 0;
	else {
		read (g_io_channel_unix_get_fd (iochannel), &video_info, sizeof (struct VideoInfo));

		if (do_searching)
			client_id |= DO_SEARCHING;
		else
			client_id &= ~DO_SEARCHING;

		write (g_io_channel_unix_get_fd (iochannel), &client_id, sizeof (int));

		sprintf(info, "frames per second: %d\tseconds per frame: %f", video_info.fps, video_info.spf);
		for (i = 0; i < COLOR_TYPES; i++) {
			sprintf(append, "\n%s\tarea: %d\n\taverage X: %d\taverage Y: %d",
					COLOR_NAME[i], video_info.area[i], video_info.aver_x[i], video_info.aver_y[i]);
			strcat(info, append);
		}
		gtk_label_set_text ((GtkLabel *) SearchResult, info);
		return 1;
	}
}

gboolean socket_frame_event(GIOChannel* iochannel, GIOCondition condition)
{
	GdkPixbuf *pixbuf;
	int client_id = GTK_GUARDER_FRAME_ID;

	if (!frame_inited) {
		if (!(frame_map = (unsigned char *) malloc (CAPTURE_WIDTH * CAPTURE_HEIGHT * 3))) {
			perror ("malloc");
			exit(-1);
		}
		frame_inited = 1;
		frame_pointer = frame_map;
	}

	if (condition & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
		return 0;
	else {
		if (frame_id == 64) { /* a whole frame has been translated */
			pixbuf = gdk_pixbuf_new_from_data ((unsigned char *) frame_map, GDK_COLORSPACE_RGB, FALSE,
					8, CAPTURE_WIDTH, CAPTURE_HEIGHT, CAPTURE_WIDTH * 3, NULL, NULL);

			if (pagenum == 0)
				gtk_image_set_from_pixbuf (GTK_IMAGE (image_info), pixbuf);
			else if (pagenum == 1)
				gtk_image_set_from_pixbuf (GTK_IMAGE (image_vision), pixbuf);

			frame_id = 0;
			frame_pointer = frame_map;
		}

		read (g_io_channel_unix_get_fd (iochannel), frame_pointer, LARGEST_DATAGRAM);

		if (pagenum == 0 || pagenum == 1)
			write (g_io_channel_unix_get_fd (iochannel), &client_id, sizeof (int));
		else
			blocked_sockfd = g_io_channel_unix_get_fd (iochannel);

		frame_pointer += LARGEST_DATAGRAM;
		frame_id++;

		return 1;
	}
}

gboolean StartStopSearching (GtkWidget *widget)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
		do_searching = 1;
		return 1;
	} else {
		do_searching = 0;
		return 1;
	}
}

gboolean Adjusted (void)
{
	int i, onestep, server_id;
	char labeltxt[50];
	struct motor_step step;

	for (i = 0; i < MOTOR_NUM; i++) {
		onestep = (int) gtk_adjustment_get_value (Adjustment[i]);
		sprintf (labeltxt, "Motor %d: %d", i + 1, onestep);
		gtk_label_set_text ((GtkLabel *) ScrollLabel[i], labeltxt);
		step.onestep[i] = onestep;
	}

	if (unlikely (!gait_inited)) {
		gait_sockfd = InitSocket (GAIT_ADJUST_ID, "Gait Adjustment", &server_id, REMOTE_ADDR);
		gait_inited = 1;
	} else if (unlikely (gait_inited == -1))
		return -1;

//	while (gait_mutex);
//	gait_mutex = 1;
	write (gait_sockfd, &step, sizeof (struct motor_step));
	read (gait_sockfd, &server_id, sizeof (int));
//	gait_mutex = 0;

	return 1;
}

gboolean PageChanged (guint page_num)
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
			break;
		case 2:
			break;
		default:
			break;
	}

	return 1;
}

gboolean SetInitStep (void)
{
	int i;
	char info[100], append[5];
	struct motor_step init;

	strcpy (info, "");
	for (i = 0; i < MOTOR_NUM; i++) {
		init.onestep[i] = (int) gtk_adjustment_get_value (Adjustment[i]);

		sprintf (append, "%d,", init.onestep[i]);
		strcat (info, append);
	}
	gtk_label_set_text ((GtkLabel *) StepInfo, info);
	RecordInitStep (init);

	return 1;
}

gboolean Reset (void)
{
	int i;

	for (i = 0; i < MOTOR_NUM; i++)
		gtk_adjustment_set_value (Adjustment[i], step_new.onestep[i]);

	Adjusted ();

	return -1;
}

gboolean EmergentReset (GtkButton *button)
{
	int server_id, i;

	if (likely (gtk_toggle_button_get_active ((GtkToggleButton *) button) == TRUE)) {
		if (unlikely (!gait_inited)) {
			for (i = 0; i < MOTOR_NUM; i++)
				gtk_adjustment_set_value (Adjustment[i], step_new.onestep[i]);
			Adjusted ();
		}
		gait_inited = -1;
		fcntl (gait_sockfd, F_SETFL, O_NONBLOCK);
		do {
			write (gait_sockfd, &step_new, sizeof (struct motor_step));
		} while (likely (!read (gait_sockfd, &server_id, sizeof (int))));
		close (gait_sockfd);
	} else
		gait_inited = 0;

	return 1;
}

gboolean LinearInterpolation (void)
{
	return 1;
}

gboolean AllLinearInterpolation (void)
{
	return 1;
}

gboolean InsertStep (void)
{
	char step[100], value[5];
	GtkTextIter start, end;
	int i;

	strcpy (step, "");
	for (i = 0; i < MOTOR_NUM; i++) {
		sprintf (value, "%d,", (int) gtk_adjustment_get_value (Adjustment[i]));
		strcat (step, value);
	}
	sprintf (value, "\n");
	strcat (step, value);

//	gtk_text_buffer_insert (GTK_TEXT_BUFFER (text_buffer), &end, step, strlen (step));
	gtk_text_buffer_insert_at_cursor (GTK_TEXT_BUFFER (text_buffer), step, strlen (step));

	return 1;
}

gboolean RemoveStep (void)
{
	return 1;
}

gboolean ClearStep (void)
{
	GtkTextIter start, end;

	gtk_text_buffer_get_bounds (GTK_TEXT_BUFFER (text_buffer), &start, &end);
	gtk_text_buffer_delete (GTK_TEXT_BUFFER (text_buffer), &start, &end);
	return 1;
}
