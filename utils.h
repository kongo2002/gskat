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

#ifndef __UTILS_H__
#define __UTILS_H__

#include "def.h"

void swap(gint *, gint *);

gint get_card_points(gint);

gchar *get_card_rank(gint, gchar *);

gchar *get_card_suit(gint, gchar *);

void print_player_cards(player *);

void print_card(card *);

gboolean higher_rank(card *, card *);

gboolean is_greater(card *, card *, gint, gboolean);

#endif /* __UTILS_H__ */

/* vim:set et sw=4 ts=4 tw=80: */
