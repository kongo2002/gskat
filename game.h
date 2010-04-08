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

void give_cards();

void give_to_skat(card *card);

gboolean play_card(GdkEventButton *event);

gboolean click_skat(GdkEventButton *event);

card *click_card(GdkEventButton *event, GList *list);

void card_to_player(player *player, card *card);

gint compare_cards(gconstpointer a, gconstpointer b, gpointer data);

gint compare_jacks(gconstpointer a, gconstpointer b);

gint compare_family(gconstpointer a, gconstpointer b);

gint get_provoke_response(gint value, gchar *msg, gboolean hoeren);

gboolean is_trump(card *card);

GList *get_jack_list(GList *list);

GList *get_suit_list(GList *list, gint suit);

GList *get_trump_list(GList *list);

GList *get_possible_cards(GList *list);

gint get_best_suit(GList *list);

gint rate_cards(player *player, GList *list);

gint get_spitzen(GList *list, gint suit);

gint get_max_reizwert(GList *list);

gint next_reizwert(gint value);

gint do_hoeren(player *player, gint value, gint sager);

gint do_sagen(player *player, gint hoerer, gint value);

void take_skat();

void start_provoke();

void calculate_stich();

void druecke_skat();

void throw_card(card *card);

void ai_play_card(player *player);

void spiel_ansagen();

void reset_game();

void end_round();

void play_stich();

void game_start();

#endif /* __GAME_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
