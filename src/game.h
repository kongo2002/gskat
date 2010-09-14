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

#ifndef __GAME_H__
#define __GAME_H__

#include "def.h"

void give_cards(void);

void give_to_skat(card *card);

gboolean play_card(GdkEventButton *event);

gboolean click_skat(GdkEventButton *event);

card *click_card(GdkEventButton *event, GList *list);

void card_to_player(player *player, card *card);

gint get_bid_response(gint value, gchar *msg, gboolean hoeren);

GList *get_possible_cards(GList *list);

gint get_best_suit(GList *list);

gint rate_cards(player *player, GList *list);

gint get_max_reizwert(GList *list);

gint next_reizwert(gint value);

gint do_hoeren(player *player, gint value, gint sager);

gint do_sagen(player *player, gint hoerer, gint value);

void take_skat(void);

void start_bidding(void);

void calculate_stich(void);

void druecke_skat(void);

gboolean throw_card(gpointer data);

void ai_play_card(player *player);

void spiel_ansagen(void);

void reset_game(void);

void set_round_points(gint winner, gint points);

void end_round(finish_type ft);

void play_stich(void);

gboolean game_abort(void);

void game_start(void);

#endif /* __GAME_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
