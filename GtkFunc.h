/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 * Some inclusions, definitions, structures, varibles, and declarations for functions.
 */

#ifndef _Gtk_Func_H_
#define _Gtk_Func_H_

#include "Common.h"
#include "BottomLayer.h"
#include <gtk/gtk.h>

GtkWidget *dialog, *SearchResult, *GaitInfo, *image_info, *image_vision, *ScrollLabel[MOTOR_NUM];
GtkWidget *point_info, *SearchButton;
GtkAdjustment *Adjustment[MOTOR_NUM];

gboolean deleted (GtkWidget *, GdkEvent *, gpointer);
gboolean socket_event (GIOChannel *, GIOCondition, gpointer);
gboolean socket_frame_event (GIOChannel *, GIOCondition, gpointer);
gboolean StartStopSearching (GtkWidget *, gpointer);
gboolean Adjusted (GtkAdjustment *, gpointer);
gboolean PageChanged (GtkNotebook *, GtkNotebookPage *, guint, gpointer);
gboolean motion_notify (GtkWidget *, GdkEventButton *, gpointer);
gboolean button_press (GtkWidget *, GdkEventButton *, gpointer);

#endif
