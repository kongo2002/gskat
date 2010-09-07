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

#include "def.h"
#include "gamestate.h"

global_state *get_current_state()
{
    global_state *state;

    if (!(state = g_malloc(sizeof(global_state))))
        return NULL;

    state->cplayer   = 0;
    state->gereizt   = 18;
    state->num_stich = 2;
    state->trump     = KREUZ;

    return state;
}

gboolean save_state_to_file(const gchar *filename)
{
    FILE *output;
    global_state *state;

    if (!(output = g_fopen(filename, "wb")))
    {
        DPRINT((_("Error on opening file '%s' for writing.\n"), filename));
        return FALSE;
    }

    if (!(state = get_current_state()))
    {
        DPRINT((_("Could not determine current game state.\n")));

        fclose(output);
        return FALSE;
    }

    if (fwrite(state, sizeof(global_state), 1, output) != 1)
    {
        DPRINT((_("Error on writing game state to file '%s'\n"), filename));

        fclose(output);
        g_free(state);
        return FALSE;
    }

    DPRINT((_("Wrote game state to file '%s'\n"), filename));

    fclose(output);
    g_free(state);
    return TRUE;
}

gboolean read_state_from_file(const gchar *filename)
{
    FILE *input;
    global_state *state;

    if (!(state = g_malloc(sizeof(global_state))))
        return FALSE;

    if (!(input = g_fopen(filename, "rb")))
    {
        DPRINT((_("Error on opening file '%s' for reading.\n"), filename));

        g_free(state);
        return FALSE;
    }

    if (fread(state, sizeof(global_state), 1, input) != 1)
    {
        DPRINT((_("Error on reading game state from file '%s'\n"), filename));

        fclose(input);
        g_free(state);
        return FALSE;
    }

    DPRINT(("%d\n%d\n%d\n%d\n", state->cplayer, state->gereizt,
            state->num_stich, state->trump));

    fclose(input);
    g_free(state);
    return TRUE;
}

/* vim:set et sw=4 sts=4 tw=80: */
