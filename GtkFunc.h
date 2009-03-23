/*
 * Copyright (C) 2007 Wang Guoqin <wangguoqin1001@gmail.com>
 * May be copied or modified under the terms of the GNU General Public License.
 *
 * Some inclusions, definitions, structures, varibles, and declarations for functions.
 * Only for My_Pickup.c right now.
 */

#ifndef _Gtk_Func_H_
#define _Gtk_Func_H_

#include "Common.h"
#include "BottomLayer.h"
#include <gtk/gtk.h>

GtkWidget *dialog, *SearchResult, *GaitInfo, *image, *ScrollLabel[MOTOR_NUM];
GtkAdjustment *Adjustment[MOTOR_NUM];

gboolean deleted (GtkWidget *, GdkEvent *, gpointer);
gboolean socket_event (GIOChannel *, GIOCondition, gpointer);
gboolean socket_frame_event (GIOChannel *, GIOCondition, gpointer);
gboolean StartStopSearching (GtkWidget *, gpointer);
gboolean Adjusted (GtkAdjustment *, gpointer);
gboolean PageChanged (GtkNotebook *, GtkNotebookPage *, guint, gpointer);

#endif
