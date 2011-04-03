/*
 *  This file is part of gskat.
 *
 *  Copyright (C) 2010-2011 by Gregor Uhlenheuer
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

G_BEGIN_DECLS

/**
 * card_state:
 * @suit:      Card suit
 * @rank:      Card rank
 * @owner:     Card owner
 * @status:    Card status
 * @draw:      Draw card?
 * @draw_face: Draw face of the card?
 *
 * Structure containing card information to be stored
 * in the state file
 */
typedef struct _card_state {
    gint suit;
    gint rank;
    gint owner;
    gint status;
    gboolean draw;
    gboolean draw_face;
} card_state;

/**
 * player_state:
 * @re:         Re player?
 * @gereizt:    Points the player has bidden
 * @points:     Current points
 * @sum_points: Sum of points
 * @num_cards:  Number of cards
 *
 * Structure containing player information to be stored
 * in the state file
 */
typedef struct _player_state {
    gboolean re;
    gint gereizt;
    gint points;
    gint sum_points;
    gint num_cards;
} player_state;

/**
 * global_state:
 * @num_played: Number of played cards
 * @num_stich:  Current trick index
 * @num_table:  Number of cards on the table
 * @cplayer:    Current player index
 * @forehand:   Current forehand player index
 * @trump:      Current trump suit
 * @hand:       Hand game
 * @null:       Null game
 * @re_player:  Re player index
 * @skat:       Cards in the skat
 * @pstates:    Array of all three #player_state objects
 *
 * Structure containing all global game information to be stored
 * in the state file
 */
typedef struct _global_state {
    gint num_played;
    gint num_stich;
    gint num_table;
    gint cplayer;
    gint forehand;
    gint trump;
    gboolean hand;
    gboolean null;
    gint re_player;
    gint skat[2];
    player_state pstates[3];
} global_state;

/**
 * state_group:
 * @gs:     #global_state object
 * @cs:     #card_state array
 * @pc:     Played cards indices
 * @pcards: Players' cards
 * @table:  Card indices on the table
 *
 * Structure containing all information to be stored
 * in the state file
 */
typedef struct _state_group {
    global_state *gs;
    card_state *cs;
    gint *pc;
    gint **pcards;
    gint *table;
} state_group;

global_state *get_global_state(void);

card_state *get_card_states(void);

gboolean save_global_state(FILE *output);

gboolean save_card_states(FILE *output);

gboolean save_played_card_states(FILE *output);

gboolean save_players_cards_state(FILE *output);

gboolean save_table_state(FILE *output);

gboolean save_state_to_file(const gchar *filename);

global_state *read_global_state(FILE *input);

card_state *read_card_states(FILE *input);

gboolean read_played_cards_state(FILE *input, state_group *sg, guint num_cards);

gboolean read_players_cards_state(FILE *input, state_group *sg,
        global_state *gs);

gboolean read_table_state(FILE *input, state_group *sg, guint num_table);

gboolean read_state_from_file(const gchar *filename);

void apply_states(state_group *sg);

void load_game(gchar *filename);

G_END_DECLS

#endif /* __GAMESTATE_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
