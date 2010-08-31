/*
 *  This file is part of gskat.
 *
 *  Copyright (C) 2010 by Gregor Uhlenheuer
 *
 *  gskat is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gskat is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gskat.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __CALLBACK_H__
#define __CALLBACK_H__

#include "def.h"

gboolean quit(GtkWidget *window, gpointer data);

gboolean destroy_show_trick(GtkWidget *widget, GdkEvent *event, gpointer data);

gboolean close_show_trick(GtkButton *button, gpointer data);

gboolean prev_stich_click(GtkButton *button, gpointer data);

gboolean next_stich_click(GtkButton *button, gpointer data);

gboolean destroy_config(GtkWidget *widget, GdkEvent *event, gpointer data);

gboolean close_config(GtkButton *button, gpointer data);

gboolean save_config(GtkButton *button, gpointer data);

void next_round(GtkButton *button, gpointer data);

gboolean configure(GtkWidget *area, GdkEventExpose *event, gpointer data);

gboolean mouse_move(GtkWidget *area, GdkEventMotion *event, gpointer data);

gboolean mouse_click(GtkWidget *area, GdkEventButton *event, gpointer data);

void show_tricks_toggle(GtkToggleButton *tbutton, gpointer data);

void animation_toggle(GtkToggleButton *tbutton, gpointer data);

gboolean realization(GtkWidget *area, gpointer data);

void refresh_tricks(GtkWidget *area, GdkEventExpose *event, gpointer data);

void refresh(GtkWidget *area, GdkEventExpose *event, gpointer data);

#endif /* __CALLBACK_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
