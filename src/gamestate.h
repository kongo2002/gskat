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

typedef struct _player_state
{
    gint points;
} player_state;

typedef struct _global_state
{
    gint num_stich;
    gint cplayer;
    gint trump;
    gint gereizt;
    /* player_state pstates[3]; */
} global_state;

global_state *get_current_state();

gboolean save_state_to_file(const gchar *filename);

gboolean read_state_from_file(const gchar *filename);

#endif

/* vim:set et sw=4 sts=4 tw=80: */
