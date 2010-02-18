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

player *init_player(gint, gchar *, gboolean);

void load_icons(app *);

void alloc_app(app *);

void load_config(app *);

void create_interface(app *);

void pos_player_cards(player *, gint, gint, gint);

void calc_card_positions(app *);

void load_card(GList **, const gchar *, gint, gint);

cairo_surface_t *load_image(gchar *);

gboolean load_cards(const gchar *, app *);

void draw_cards(app *, GList *, cairo_t *);

void draw_player(app *, player *, cairo_t *);

void draw_area(app *);

void free_app(app *);

#endif /* __INTERFACE_H__ */

/* vim:set et sw=4 ts=4 tw=80: */
