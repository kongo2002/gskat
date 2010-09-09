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

#ifndef __GAMESTATE_H__
#define __GAMESTATE_H__

typedef struct _card_state
{
    /** Card suit */
    gint suit;
    /** Card rank */
    gint rank;
    /** Owner */
    gint owner;
    /** Card status */
    gint status;
    /** Draw card? */
    gboolean draw;
    /** Draw face of the card? */
    gboolean draw_face;
} card_state;

typedef struct _player_state
{
    /** Re player? */
    gboolean re;
    /** Points the player has bidden */
    gint gereizt;
    /** Current points */
    gint points;
    /** Sum of points */
    gint sum_points;
    /** Number of cards */
    gint num_cards;
} player_state;

typedef struct _global_state
{
    /** Number of played cards */
    gint num_played;
    /** Current trick index */
    gint num_stich;
    /** Number of cards on the table */
    gint num_table;
    /** Current player index */
    gint cplayer;
    /** Current forehand player index */
    gint forehand;
    /** Current trump suit */
    gint trump;
    /** Hand game */
    gboolean hand;
    /** Null game */
    gboolean null;
    /** Re player index */
    gint re_player;
    /** Cards in the skat */
    gint skat[2];
    /** Array of all three player states */
    player_state pstates[3];
} global_state;

typedef struct _state_group
{
    /** Global state structure */
    global_state *gs;
    /** Card states array */
    card_state *cs;
    /** Played cards indices */
    gint *pc;
    /** Players' cards */
    gint **pcards;
    /** Cards on the table */
    gint *table;
} state_group;

global_state *get_global_state();

card_state *get_card_states();

gboolean save_global_state(FILE *output);

gboolean save_card_states(FILE *output);

gboolean save_played_card_states(FILE *output);

gboolean save_players_cards_state(FILE *output);

gboolean save_table_state(FILE *output);

gboolean save_state_to_file(const gchar *filename);

global_state *read_global_state(FILE *input);

card_state *read_card_states(FILE *input);

gint *read_played_cards_state(FILE *input, gint num_cards);

gboolean read_players_cards_state(FILE *input, state_group *sg,
        global_state *gs);

gboolean read_table_state(FILE *input, state_group *sg, gint num_table);

gboolean read_state_from_file(const gchar *filename);

void apply_states(state_group *sg);

#endif /* __GAMESTATE_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
