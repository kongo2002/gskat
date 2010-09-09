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
} player_state;

typedef struct _global_state
{
    /** Current trick index */
    gint num_stich;
    /** Current player index */
    gint cplayer;
    /** Current trump suit */
    gint trump;
    /** Hand game */
    gboolean hand;
    /** Null game */
    gboolean null;
    /** Re player index */
    gint re_player;
    /** Array of all three player states */
    player_state pstates[3];
} global_state;

typedef struct _state_group
{
    /** Global state structure */
    global_state *gs;
    /** Card states array */
    card_state *cs;
    /** Number of played cards */
    gint num_played;
    /** Played cards indices */
    gint *pc;
} state_group;

global_state *get_global_state();

gboolean save_state_to_file(const gchar *filename);

gboolean read_state_from_file(const gchar *filename);

void apply_states(state_group *sg);

#endif /* __GAMESTATE_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
