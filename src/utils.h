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

gint get_card_points(gint rank);

card *get_card_by_id(gint id);

card *get_card_ptr(gint suit, gint rank);

gchar *suit_name(gint id);

gchar *rank_name(gint id);

gchar *get_card_name(card *card);

void print_player_cards(player *player);

void print_card(card *card);

gboolean higher_rank(card *top, card *bottom);

gboolean is_greater(card *top, card *bottom);

gint get_trick_winner(card **trick);

gint get_table_winner(void);

gint get_game_multiplier(void);

gint get_game_base_value(player *re);

gint get_spitzen(GList *list, gint suit);

gboolean is_trump(card *card);

GList *get_trump_list(GList *list);

GList *get_jack_list(GList *list);

GList *get_suit_list(GList *list, gint suit);

gint get_suit_len(GList *list, gint suit);

gint num_different_suits(GList *list);

gint compare_cards(gconstpointer a, gconstpointer b);

gint compare_jacks(gconstpointer a, gconstpointer b);

gint compare_family(gconstpointer a, gconstpointer b);

#endif /* __UTILS_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
