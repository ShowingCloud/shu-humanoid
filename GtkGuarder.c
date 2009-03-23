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
	GtkWidget *hbox1, *hbox2, *vbox1, *vbox2, *vbox3, *vbox4, *ImageFrame, *SearchResultFrame, *ScrollBar[MOTOR_NUM];
	GtkWidget *GaitInfoFrame, *notebook, *testinfo, *SearchButton, *AnalyseFrame, *GaitAdjustFrame, *label;
	GtkWidget *text, *button;
	GdkPixbuf *pixbuf;

	int sock_fd, sock_frame_fd, server_id, i;
	char info[500], append[300], labeltxt[50];
	GIOChannel *io_channel, *io_frame_channel;

	sock_fd = InitSocket (GTK_GUARDER_ID, "Gtk Guarder", &server_id, REMOTE_ADDR);

	gtk_init (&argc, &argv);
	dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (dialog), "GTK Guarder");
//	gtk_widget_set_size_request (GTK_WIDGET (dialog), 768, 640);

	hbox1 = gtk_hbox_new (FALSE, 5);
	vbox1 = gtk_vbox_new (FALSE, 5);
	vbox2 = gtk_vbox_new (FALSE, 5);

	ImageFrame = gtk_frame_new ("Image");
	SearchResultFrame = gtk_frame_new ("SearchResult");
	GaitInfoFrame = gtk_frame_new ("Gait Info");

	notebook = gtk_notebook_new ();
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_LEFT);

	g_signal_connect(G_OBJECT(dialog), "delete_event", G_CALLBACK (gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK (gtk_main_quit), NULL);

	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, CAPTURE_WIDTH, CAPTURE_HEIGHT);
	image_info = gtk_image_new_from_pixbuf (pixbuf);
	gtk_container_add (GTK_CONTAINER (ImageFrame), image_info);
	gtk_box_pack_start (GTK_BOX (vbox1), ImageFrame, FALSE, FALSE, 0);

	sprintf (info, "frames per second: N/A\tseconds per frame: N/A");
	for (i = 0; i < COLOR_TYPES; i++) {
		sprintf(append, "\n%s\tarea: N/A\n\taverage X: N/A\taverage Y: N/A", COLOR_NAME[i]);
		strcat(info, append);
	}
	SearchResult = gtk_label_new(info);
	gtk_container_add (GTK_CONTAINER (SearchResultFrame), SearchResult);
	gtk_box_pack_start (GTK_BOX (vbox2), SearchResultFrame, FALSE, FALSE, 0);

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
	gtk_container_add (GTK_CONTAINER (GaitInfoFrame), GaitInfo);
	gtk_box_pack_start (GTK_BOX (vbox2), GaitInfoFrame, FALSE, FALSE, 0);

	SearchButton = gtk_check_button_new_with_label ("Do Searching");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (SearchButton), FALSE);
	g_signal_connect(G_OBJECT (SearchButton), "toggled", G_CALLBACK (StartStopSearching), NULL);
	gtk_box_pack_start (GTK_BOX (vbox2), SearchButton, FALSE, FALSE, 0);

	gtk_container_add (GTK_CONTAINER (hbox1), vbox1);
	gtk_container_add (GTK_CONTAINER (hbox1), vbox2);

	label = gtk_label_new ("Information");
	gtk_label_set_angle ((GtkLabel *) label, 90);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox1, label);

	hbox1 = gtk_hbox_new (FALSE, 5);
	vbox1 = gtk_vbox_new (FALSE, 5);
	vbox2 = gtk_vbox_new (FALSE, 5);

	ImageFrame = gtk_frame_new ("Image");
	AnalyseFrame = gtk_frame_new ("Vision Analyse");

	image_vision = gtk_image_new_from_pixbuf (pixbuf);
	gtk_container_add (GTK_CONTAINER (ImageFrame), image_vision);
	gtk_box_pack_start (GTK_BOX (vbox1), ImageFrame, FALSE, FALSE, 0);

	testinfo = gtk_label_new ("Vision Analyse");
	gtk_container_add (GTK_CONTAINER (AnalyseFrame), testinfo);
	gtk_box_pack_start (GTK_BOX (vbox2), AnalyseFrame, FALSE, FALSE, 0);

	gtk_container_add (GTK_CONTAINER (hbox1), vbox1);
	gtk_container_add (GTK_CONTAINER (hbox1), vbox2);

	label = gtk_label_new ("Vision");
	gtk_label_set_angle ((GtkLabel *) label, 90);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox1, label);

	hbox1 = gtk_hbox_new (FALSE, 5);
	hbox2 = gtk_hbox_new (FALSE, 5);
	vbox1 = gtk_vbox_new (FALSE, 5);
	vbox2 = gtk_vbox_new (FALSE, 5);
	vbox3 = gtk_vbox_new (FALSE, 5);
	vbox4 = gtk_vbox_new (FALSE, 5);

	GaitAdjustFrame = gtk_frame_new ("Gait Adjustment");

	step_new = ReadInitStep ();
	if (unlikely(step_new.onestep[0] == 0 && step_new.onestep[1] == 0)) {
		printf ("Didn't get initial step from config file. Using default.");
		step_new = step_init;
	}

	strcpy (info, "");
	for (i = 0; i < MOTOR_NUM; i++) {
		Adjustment[i] = (GtkAdjustment *) gtk_adjustment_new (step_new.onestep[i], 0, 180, 1, 5, 0);
		g_signal_connect (G_OBJECT (Adjustment[i]), "value_changed", G_CALLBACK (Adjusted), NULL);
		ScrollBar[i] = gtk_hscrollbar_new (Adjustment[i]);

		sprintf (labeltxt, "Motor %d: %d", i + 1, (int) gtk_adjustment_get_value (Adjustment[i]));
		ScrollLabel[i] = gtk_label_new (labeltxt);
		gtk_box_pack_start (GTK_BOX ((i % 2) ? vbox2 : vbox1), ScrollLabel[i], FALSE, FALSE, 0);
		gtk_box_pack_start (GTK_BOX ((i % 2) ? vbox2 : vbox1), ScrollBar[i], FALSE, FALSE, 0);

		sprintf (append, "%d,", step_new.onestep[i]);
		strcat (info, append);
	}

	gtk_container_add (GTK_CONTAINER (hbox1), vbox1);
	gtk_container_add (GTK_CONTAINER (hbox1), vbox2);

	StepInfo = gtk_label_new (info);
	gtk_box_pack_start (GTK_BOX (vbox3), StepInfo, FALSE, FALSE, 0);
	text = gtk_text_view_new ();
	text_buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	gtk_widget_set_size_request (text, 300, 500);
	gtk_text_view_set_editable ((GtkTextView *) text, FALSE);
//	gtk_text_view_set_cursor_visible ((GtkTextView *) text, FALSE);
	gtk_box_pack_start (GTK_BOX (vbox3), text, FALSE, FALSE, 0);

	gtk_widget_set_size_request (vbox3, 300, 600);
	gtk_container_add (GTK_CONTAINER (hbox1), vbox3);
	gtk_container_add (GTK_CONTAINER (vbox4), hbox1);

	button = gtk_button_new_with_label ("Set Init Step");
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (SetInitStep), NULL);
	gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);
	button = gtk_button_new_with_label ("Reset");
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (Reset), NULL);
	gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);
	button = gtk_toggle_button_new_with_label ("Emergent Reset");
	g_signal_connect (G_OBJECT (button), "toggled", G_CALLBACK (EmergentReset), NULL);
	gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);
	button = gtk_button_new_with_label ("Linear Interpolation");
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (LinearInterpolation), NULL);
	gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);
	button = gtk_button_new_with_label ("All Linear Interpolation");
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (AllLinearInterpolation), NULL);
	gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);
	button = gtk_button_new_with_label ("Insert a Step");
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (InsertStep), NULL);
	gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);
	button = gtk_button_new_with_label ("Remove a Step");
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (RemoveStep), NULL);
	gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);
	button = gtk_button_new_with_label ("Clear All");
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (ClearStep), NULL);
	gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE, FALSE, 0);
	gtk_container_add (GTK_CONTAINER (vbox4), hbox2);
	gtk_container_add (GTK_CONTAINER (GaitAdjustFrame), vbox4);

	label = gtk_label_new ("Gait");
	gtk_label_set_angle ((GtkLabel *) label, 90);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), GaitAdjustFrame, label);

	g_signal_connect (G_OBJECT (notebook), "switch-page", G_CALLBACK (PageChanged), NULL);
	gtk_container_add(GTK_CONTAINER(dialog), notebook);

	io_channel = g_io_channel_unix_new (sock_fd);
	g_io_channel_set_encoding (io_channel, NULL, NULL);
	g_io_add_watch (io_channel, (GIOCondition) (G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL), (GIOFunc) socket_event, NULL);

	sock_frame_fd = InitSocket (GTK_GUARDER_FRAME_ID, "Gtk Guarder Frame", &server_id, REMOTE_ADDR);

	io_frame_channel = g_io_channel_unix_new (sock_frame_fd);
	g_io_channel_set_encoding (io_frame_channel, NULL, NULL);
	g_io_add_watch (io_frame_channel, (GIOCondition) (G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL),
			(GIOFunc) socket_frame_event, NULL);

	gtk_widget_show_all(dialog);

	gtk_main();

//	gtk_widget_destroy(dialog);
	g_io_channel_shutdown (io_channel, TRUE, NULL);
	g_io_channel_shutdown (io_frame_channel, TRUE, NULL);
	close (sock_fd);
	close (sock_frame_fd);

	return 0;
}
