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

void give_cards(app *);

void give_to_skat(app *, card *);

gboolean play_card(app *, GdkEventButton *);

gboolean click_skat(app *, GdkEventButton *);

card *click_card(app *, GdkEventButton *, GList *);

void card_to_player(player *, card *);

gint compare_cards(gconstpointer, gconstpointer, gpointer);

gint compare_jacks(gconstpointer, gconstpointer);

gint compare_family(gconstpointer, gconstpointer);

gint get_provoke_response(app *, gint, gchar *, gboolean);

gboolean is_trump(app *, card *);

GList *get_jack_list(GList *);

GList *get_suit_list(app *, GList *, gint);

GList *get_trump_list(app *, GList *);

GList *get_possible_cards(app *, GList *);

gint get_best_suit(GList *);

gint rate_cards(app *, player *, GList *);

gint get_spitzen(app *, GList *, gint);

gint get_max_reizwert(app *, GList *);

gint next_reizwert(gint);

gint do_hoeren(app *, player *, gint, gint);

gint do_sagen(app *, player *, gint, gint);

void take_skat(app *);

void start_provoke(app *app);

void calculate_stich(app *app);

void druecke_skat(app *);

void throw_card(app *, card *);

void ai_play_card(app *, player *);

void spiel_ansagen(app *);

void reset_game(app *);

void end_round(app *);

void play_stich(app *);

void game_start(app *);

#endif /* __GAME_H__ */

/* vim:set et sw=4 sts=4 ts=8 tw=80: */
