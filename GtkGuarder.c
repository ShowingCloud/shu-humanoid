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
#include <time.h>
#include <sys/time.h>
#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "GtkGuarder.h"

GtkWidget *dialog, *SearchResult, *GaitInfo, *image;

int main(int argc, char** argv)
{
	GtkWidget *hbox, *vbox1, *vbox2, *ImageFrame, *SearchResultFrame, *GaitInfoFrame, *notebook, *testinfo, *SearchButton;
	GdkPixbuf *pixbuf;

	int sock_fd, sock_frame_fd, result, i, sock_id = GTK_GUARDER_ID, sock_frame_id = GTK_GUARDER_FRAME_ID, server_id;
	socklen_t len;
	struct sockaddr_in address;
	char info[500], append[300];
	GIOChannel *io_channel, *io_frame_channel;
	
	sock_fd = socket (AF_INET, SOCK_STREAM, 0);
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr ("127.0.0.1");
	address.sin_port = htons (10200);
	len = sizeof (address);
	
	if ((result = connect (sock_fd, (struct sockaddr *) &address, len)) == -1)
	{
		perror ("oops: GtkGuarder");
		exit(-1);
	}

	write (sock_fd, &sock_id, sizeof (int));
	read (sock_fd, &server_id, sizeof (int));
	if ((server_id & ID_MASK) != SOCKET_LISTENER_ID)
	{
		printf ("Error: unknown socket server!\n");
		exit(-1);
	}
	else
		printf ("Connected with the socket server!\n");

	gtk_init (&argc, &argv);
	dialog = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (dialog), "GTK Guarder");
//	gtk_widget_set_size_request (GTK_WIDGET (dialog), 768, 640);

	hbox = gtk_hbox_new (FALSE, 5);
	vbox1 = gtk_vbox_new (FALSE, 5);
	vbox2 = gtk_vbox_new (FALSE, 5);

	ImageFrame = gtk_frame_new ("Image");
	SearchResultFrame = gtk_frame_new ("SearchResult");
	GaitInfoFrame = gtk_frame_new ("Gait Info");

	notebook = gtk_notebook_new ();
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);

	g_signal_connect(G_OBJECT(dialog), "delete_event", G_CALLBACK (gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK (gtk_main_quit), NULL);

	pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, CAPTURE_WIDTH, CAPTURE_HEIGHT);
	image = gtk_image_new_from_pixbuf (pixbuf);
	gtk_container_add (GTK_CONTAINER (ImageFrame), image);
	gtk_box_pack_start (GTK_BOX (vbox1), ImageFrame, FALSE, FALSE, 0);

	sprintf (info, "frames per second: N/A\tseconds per frame: N/A");
	for (i = 0; i < COLOR_TYPES; i++)
	{
		sprintf(append, "\n%s\tarea: N/A\n\taverage X: N/A\taverage Y: N/A", COLOR_NAME[i]);
		strcat(info, append);
	}
	SearchResult = gtk_label_new(info);
	gtk_container_add (GTK_CONTAINER (SearchResultFrame), SearchResult);
	gtk_box_pack_start (GTK_BOX (vbox2), SearchResultFrame, FALSE, FALSE, 0);

	strcpy (info, "");
	for (i = 1; i <= 24; i++)
	{
		sprintf (append, "motor %d: N/A", i);
		strcat (info, append);

		if (i != 24)
		{
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
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (SearchButton), TRUE);
	g_signal_connect(G_OBJECT (SearchButton), "toggled", G_CALLBACK (StartStopSearching), NULL);
	gtk_box_pack_start (GTK_BOX (vbox2), SearchButton, FALSE, FALSE, 0);

	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox2, gtk_label_new ("Information"));

	vbox2 = gtk_vbox_new (FALSE, 5);
	testinfo = gtk_label_new ("test information");
	gtk_box_pack_start (GTK_BOX (vbox2), testinfo, FALSE, FALSE, 0);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), vbox2, gtk_label_new ("test"));

	gtk_container_add (GTK_CONTAINER (hbox), vbox1);
	gtk_container_add (GTK_CONTAINER (hbox), notebook);

	gtk_container_add(GTK_CONTAINER(dialog), hbox);

	io_channel = g_io_channel_unix_new (sock_fd);
	g_io_channel_set_encoding (io_channel, NULL, NULL);
	g_io_add_watch (io_channel, (GIOCondition) (G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL), socket_event, NULL);

	sock_frame_fd = socket (AF_INET, SOCK_STREAM, 0);
	if ((result = connect (sock_frame_fd, (struct sockaddr *) &address, len)) == -1)
	{
		perror ("oops: GtkGuarder");
		exit(-1);
	}

	write (sock_frame_fd, &sock_frame_id, sizeof (int));
	read (sock_frame_fd, &server_id, sizeof (int));
	if ((server_id & ID_MASK) != SOCKET_LISTENER_ID)
	{
		printf ("Error: unknown socket server!\n");
		exit(-1);
	}
	else
		printf ("Connected with the socket server! (frame)\n");

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
