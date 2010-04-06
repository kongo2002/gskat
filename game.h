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

void give_cards(app *app);

void give_to_skat(app *app, card *card);

gboolean play_card(app *app, GdkEventButton *event);

gboolean click_skat(app *app, GdkEventButton *event);

card *click_card(app *app, GdkEventButton *event, GList *list);

void card_to_player(player *player, card *card);

gint compare_cards(gconstpointer a, gconstpointer b, gpointer data);

gint compare_jacks(gconstpointer a, gconstpointer b);

gint compare_family(gconstpointer a, gconstpointer b);

gint get_provoke_response(app *app, gint value, gchar *msg, gboolean hoeren);

gboolean is_trump(app *app, card *card);

GList *get_jack_list(GList *list);

GList *get_suit_list(app *app, GList *list, gint suit);

GList *get_trump_list(app *app, GList *list);

GList *get_possible_cards(app *app, GList *list);

gint get_best_suit(GList *list);

gint rate_cards(app *app, player *player, GList *list);

gint get_spitzen(app *app, GList *list, gint suit);

gint get_max_reizwert(app *app, GList *list);

gint next_reizwert(gint value);

gint do_hoeren(app *app, player *player, gint value, gint sager);

gint do_sagen(app *app, player *player, gint hoerer, gint value);

void take_skat(app *app);

void start_provoke(app *app);

void calculate_stich(app *app);

void druecke_skat(app *app);

void throw_card(app *app, card *card);

void ai_play_card(app *app, player *player);

void spiel_ansagen(app *app);

void reset_game(app *app);

void end_round(app *app);

void play_stich(app *app);

void game_start(app *app);

#endif /* __GAME_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
