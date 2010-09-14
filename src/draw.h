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

#ifndef __DRAW_H__
#define __DRAW_H__

void pos_player_cards(player *player, gint x, gint y, gint step);

void calc_card_positions(void);

void set_table_position(card *card, gint *dest_x, gint *dest_y);

void set_card_move_step(card_move *cm);

gboolean move_card(gpointer data);

void draw_cards(GList *cards, cairo_t *target);

void draw_player(player *player, cairo_t *cr);

void draw_table(GtkWidget *area, cairo_t *cr);

void draw_area(void);

void draw_tricks_area(GtkWidget *area, stich_view *sv);

#endif /* __DRAW_H__ */
