/*
 *  This file is part of gskat.
 *
 *  Copyright (C) 2010-2011 by Gregor Uhlenheuer
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

#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include "def.h"

G_BEGIN_DECLS

#define GSKAT_CONTEXTID 42

player *init_player(gint id, gchar *name, gboolean human);

void load_suit_icons(void);

void set_icons(void);

void alloc_app(void);

void show_dialog(GtkMessageType type, GtkButtonsType buttons,
        const gchar *fmt, va_list args);

void update_sb(const gchar *fmt, va_list args);

void show_file_bugreport(void);

void show_dialog_info(const gchar *fmt, va_list args);

void show_dialog_error(const gchar *fmt, va_list args);

void show_bid_infobar(gint value, gchar *msg, gboolean hoeren);

void add_summary_row(GtkTreeStore **tree, const char *key, const char *value);

GtkWidget *create_game_summary(const gchar *description, GtkTreeStore **tree);

void show_last_tricks(void);

void show_config_window(void);

void update_interface(void);

void update_rank_interface(void);

void create_interface(void);

void load_card(GList **list, const gchar *file, gint rank, gint suit);

cairo_surface_t *load_image(gchar *filename);

gboolean load_cards(const gchar **paths);

GtkWidget *get_widget(const gchar *name);

void free_app(void);

G_END_DECLS

#endif /* __INTERFACE_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
