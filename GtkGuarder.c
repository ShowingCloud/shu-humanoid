/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <gtk/gtk.h>

#include "GtkGuarder.h"
#include "SocketServer.h"
#include "BottomLayer.h"
#include "GtkFunc.h"

int main(int argc, char** argv)
{
	GtkWidget *hbox, *vbox1, *vbox2, *ImageFrame, *SearchResultFrame, *ScrollBar[MOTOR_NUM];
	GtkWidget *GaitInfoFrame, *notebook, *AnalyseFrame, *GaitAdjustFrame, *label;
	GtkWidget *event_box, *eventbox_alignment, *frame_label;
	PangoFontDescription *font;
	GdkPixbuf *pixbuf;

	int sock_fd, sock_frame_fd, server_id, i;
	char info[500], append[300], labeltxt[50];
	GIOChannel *io_channel, *io_frame_channel;

	sock_fd = InitSocket (GTK_GUARDER_ID, "Gtk Guarder", &server_id, REMOTE_ADDR, SOCKET_TCP, 0);

	gtk_init (&argc, &argv);
	dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (dialog), "GTK Guarder");
	gtk_widget_set_size_request (GTK_WIDGET (dialog), CAPTURE_WIDTH + 380, CAPTURE_HEIGHT + 50);

	g_signal_connect(G_OBJECT(dialog), "delete_event", G_CALLBACK (gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK (gtk_main_quit), NULL);

	font = pango_font_description_new ();
	pango_font_description_set_family (font, "Sans Serif");
	pango_font_description_set_size (font, 9 * PANGO_SCALE);

	notebook = gtk_notebook_new ();
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_LEFT);

	/* The first notebook -- Information */

	hbox = gtk_hbox_new (FALSE, 5);
	vbox1 = gtk_vbox_new (FALSE, 5);
	vbox2 = gtk_vbox_new (FALSE, 5);

	/* Image Frame */

	frame_label = gtk_label_new ("Image");
	gtk_widget_modify_font (GTK_WIDGET (frame_label), font);
	ImageFrame = gtk_frame_new ("");
	gtk_frame_set_label_widget (GTK_FRAME (ImageFrame), frame_label);

	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, CAPTURE_WIDTH, CAPTURE_HEIGHT);
	image_info = gtk_image_new_from_pixbuf (pixbuf);
	gtk_container_add (GTK_CONTAINER (ImageFrame), image_info);
	gtk_box_pack_start (GTK_BOX (vbox1), ImageFrame, FALSE, FALSE, 0);

	/* Search Result Frame */

	frame_label = gtk_label_new ("SearchResult");
	gtk_widget_modify_font (GTK_WIDGET (frame_label), font);
	SearchResultFrame = gtk_frame_new ("");
	gtk_frame_set_label_widget (GTK_FRAME (SearchResultFrame), frame_label);

	sprintf (info, "frames per second: N/A\tseconds per frame: N/A");
	for (i = 0; i < COLOR_TYPES; i++) {
		sprintf (append, "\n%s\tarea: N/A\n\taverage X: N/A\taverage Y: N/A", COLOR_NAME[i]);
		strcat (info, append);
	}
	SearchResult = gtk_label_new (info);
	gtk_widget_modify_font (GTK_WIDGET (SearchResult), font);
	gtk_container_add (GTK_CONTAINER (SearchResultFrame), SearchResult);
	gtk_box_pack_start (GTK_BOX (vbox2), SearchResultFrame, FALSE, FALSE, 0);

	/* Gait Info Frame */

	frame_label = gtk_label_new ("Gait Info");
	gtk_widget_modify_font (GTK_WIDGET (frame_label), font);
	GaitInfoFrame = gtk_frame_new ("");
	gtk_frame_set_label_widget (GTK_FRAME (GaitInfoFrame), frame_label);

	strcpy (info, "");
	for (i = 1; i <= 24; i++) {
		sprintf (append, "motor %d: N/A", i);
		strcat (info, append);

		if (i != 24) {
			if (!(i % 3))
				sprintf (append, "\n");
			else
				sprintf (append, "\t");
			strcat (info, append);
		}
	}
	GaitInfo = gtk_label_new (info);
	gtk_widget_modify_font (GTK_WIDGET (GaitInfo), font);
	gtk_container_add (GTK_CONTAINER (GaitInfoFrame), GaitInfo);
	gtk_box_pack_start (GTK_BOX (vbox2), GaitInfoFrame, FALSE, FALSE, 0);

	/* Do Searching Check Button */

	SearchButton = gtk_check_button_new_with_label ("Do Searching");
	gtk_widget_modify_font (GTK_WIDGET (SearchButton), font);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (SearchButton), TRUE);
	g_signal_connect (G_OBJECT (SearchButton), "toggled", G_CALLBACK (StartStopSearching), NULL);
	gtk_box_pack_start (GTK_BOX (vbox2), SearchButton, FALSE, FALSE, 0);

	gtk_container_add (GTK_CONTAINER (hbox), vbox1);
	gtk_container_add (GTK_CONTAINER (hbox), vbox2);

	/* notebook page */

	label = gtk_label_new ("Information");
	gtk_widget_modify_font (GTK_WIDGET (label), font);
	gtk_label_set_angle ((GtkLabel *) label, 90);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox, label);

	/* The second notebook -- Vision */

	hbox = gtk_hbox_new (FALSE, 5);
	vbox1 = gtk_vbox_new (FALSE, 5);
	vbox2 = gtk_vbox_new (FALSE, 5);

	/* Image Frame */

	frame_label = gtk_label_new ("Image");
	gtk_widget_modify_font (GTK_WIDGET (frame_label), font);
	ImageFrame = gtk_frame_new ("");
	gtk_frame_set_label_widget (GTK_FRAME (ImageFrame), frame_label);

	eventbox_alignment = gtk_alignment_new (0, 0, 0, 0);
	gtk_container_add (GTK_CONTAINER (ImageFrame), eventbox_alignment);
	event_box = gtk_event_box_new ();
	gtk_container_add (GTK_CONTAINER (eventbox_alignment), event_box);

	image_vision = gtk_image_new_from_pixbuf (pixbuf);
	gtk_container_add (GTK_CONTAINER (event_box), image_vision);
	gtk_box_pack_start (GTK_BOX (vbox1), ImageFrame, FALSE, FALSE, 0);

	/* Vision Analyse Frame */

	frame_label = gtk_label_new ("Vision Analyse");
	gtk_widget_modify_font (GTK_WIDGET (frame_label), font);
	AnalyseFrame = gtk_frame_new ("");
	gtk_frame_set_label_widget (GTK_FRAME (AnalyseFrame), frame_label);

	sprintf (info, "Point Property:\nX: N/A\t\tY: N/A\nR: N/A\t\tG: N/A\t\tB: N/A\nH: N/A\t\tS: N/A\t\tV: N/A");
	point_info = gtk_label_new (info);
	gtk_widget_modify_font (GTK_WIDGET (point_info), font);
	gtk_container_add (GTK_CONTAINER (AnalyseFrame), point_info);
	gtk_box_pack_start (GTK_BOX (vbox2), AnalyseFrame, FALSE, FALSE, 0);

	gtk_widget_set_events (event_box, GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK);
	g_signal_connect (G_OBJECT (event_box), "motion_notify_event", G_CALLBACK (motion_notify), NULL);
	g_signal_connect (G_OBJECT (event_box), "button_press_event", G_CALLBACK (button_press), NULL);

	gtk_container_add (GTK_CONTAINER (hbox), vbox1);
	gtk_container_add (GTK_CONTAINER (hbox), vbox2);

	/* notebook page */

	label = gtk_label_new ("Vision");
	gtk_widget_modify_font (GTK_WIDGET (label), font);
	gtk_label_set_angle ((GtkLabel *) label, 90);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox, label);

	/* The third notebook -- Gait */

	hbox = gtk_hbox_new (FALSE, 5);
	vbox1 = gtk_vbox_new (FALSE, 5);
	vbox2 = gtk_vbox_new (FALSE, 5);

	/* Gait Adjustment Frame */

	frame_label = gtk_label_new ("Gait Adjustment");
	gtk_widget_modify_font (GTK_WIDGET (frame_label), font);
	GaitAdjustFrame = gtk_frame_new ("");
	gtk_frame_set_label_widget (GTK_FRAME (GaitAdjustFrame), frame_label);

	for (i = 0; i < MOTOR_NUM; i++) {
		Adjustment[i] = (GtkAdjustment *) gtk_adjustment_new (step_init.onestep[i], 0, 180, 1, 0, 0);
		g_signal_connect (G_OBJECT (Adjustment[i]), "value_changed", G_CALLBACK (Adjusted), NULL);
		ScrollBar[i] = gtk_hscrollbar_new (Adjustment[i]);

		sprintf (labeltxt, "Motor %d: %d", i, (int) gtk_adjustment_get_value (Adjustment[i]));
		ScrollLabel[i] = gtk_label_new (labeltxt);
		gtk_widget_modify_font (GTK_WIDGET (ScrollLabel[i]), font);
		gtk_box_pack_start (GTK_BOX ((i % 2) ? vbox2 : vbox1), ScrollLabel[i], FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX ((i % 2) ? vbox2 : vbox1), ScrollBar[i], FALSE, FALSE, 0);
	}

	gtk_container_add (GTK_CONTAINER (hbox), vbox1);
	gtk_container_add (GTK_CONTAINER (hbox), vbox2);

	gtk_container_add (GTK_CONTAINER (GaitAdjustFrame), hbox);

	/* notebook page */

	label = gtk_label_new ("Gait");
	gtk_widget_modify_font (GTK_WIDGET (label), font);
	gtk_label_set_angle ((GtkLabel *) label, 90);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), GaitAdjustFrame, label);

	g_signal_connect (G_OBJECT (notebook), "switch-page", G_CALLBACK (PageChanged), NULL);
	gtk_container_add(GTK_CONTAINER(dialog), notebook);

	/* IO Channels */

	io_channel = g_io_channel_unix_new (sock_fd);
	g_io_channel_set_encoding (io_channel, NULL, NULL);
	g_io_add_watch (io_channel, (GIOCondition) (G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL), socket_event, NULL);

	sock_frame_fd = InitSocket (GTK_GUARDER_FRAME_ID, "Gtk Guarder Frame", &server_id, REMOTE_ADDR, SOCKET_UDP, 1);

	io_frame_channel = g_io_channel_unix_new (sock_frame_fd);
	g_io_channel_set_encoding (io_frame_channel, NULL, NULL);
	g_io_add_watch (io_frame_channel, (GIOCondition) (G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
			socket_frame_event, NULL);

	gtk_widget_show_all(dialog);

	gtk_main();

//	gtk_widget_destroy(dialog);
	g_io_channel_shutdown (io_channel, TRUE, NULL);
	g_io_channel_shutdown (io_frame_channel, TRUE, NULL);
	close (sock_fd);
	close (sock_frame_fd);

	return 0;
}
