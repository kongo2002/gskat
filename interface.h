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

#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include "def.h"

player *init_player(gint id, gchar *name, gboolean human);

void load_icons(app *app);

void alloc_app(app *app);

void load_config(app *app);

void create_interface(app *app);

void pos_player_cards(player *player, gint x, gint y, gint step);

void calc_card_positions(app *app);

void load_card(GList **list, const gchar *file, gint rank, gint suit);

cairo_surface_t *load_image(gchar *filename);

gboolean load_cards(const gchar *path, app *app);

void draw_cards(app *app, GList *cards, cairo_t *target);

void draw_player(app *app, player *player, cairo_t *cr);

void draw_area(app *app);

void free_app(app *app);

#endif /* __INTERFACE_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
