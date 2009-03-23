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
#include "ConfigFiles.h"
#include <gtk/gtk.h>
#include <fcntl.h>

GtkWidget *dialog, *SearchResult, *GaitInfo, *image_info, *image_vision, *ScrollLabel[MOTOR_NUM], *StepInfo;
GtkTextBuffer *text_buffer;
GtkAdjustment *Adjustment[MOTOR_NUM];
struct motor_step step_new;

gboolean deleted (GtkWidget *, GdkEvent *);
gboolean socket_event (GIOChannel *, GIOCondition);
gboolean socket_frame_event (GIOChannel *, GIOCondition);
gboolean StartStopSearching (GtkWidget *);
gboolean Adjusted (void);
gboolean PageChanged (guint);
gboolean SetInitStep (void);
gboolean Reset (void);
gboolean EmergentReset (GtkButton *);
gboolean LinearInterpolation (void);
gboolean AllLinearInterpolation (void);
gboolean InsertStep (void);
gboolean RemoveStep (void);
gboolean ClearStep (void);

#endif
