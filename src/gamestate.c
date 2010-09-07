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

/**
 * @brief Determine the current global game states like round, stich
 * and players' points
 *
 * @return new global_state* structure
 */
global_state *get_global_state()
{
    gint i;
    global_state *state;
    player_state *pstate;
    player *cur_player;

    if (!(state = g_malloc(sizeof(global_state))))
        return NULL;

    /* iterate over all three players */
    for (i=0; i<3; ++i)
    {
        pstate = &state->pstates[i];
        cur_player = gskat.players[i];

        pstate->gereizt = cur_player->gereizt;
        pstate->re = cur_player->re;
        pstate->points = cur_player->points;
        pstate->sum_points = cur_player->sum_points;
    }

    /* get global values */
    state->cplayer   = gskat.cplayer;
    state->num_stich = gskat.stich;
    state->trump     = gskat.trump;
    state->re_player = (gskat.re) ? gskat.re->id : -1;
    state->null      = gskat.null;
    state->hand      = gskat.hand;

    return state;
}

/**
 * @brief Determine the current card states
 *
 * @return an array of 32 card_state elements
 */
card_state *get_card_states()
{
    gint i = 0;
    GList *ptr;
    card *tmp;
    card_state *cards, *cstate;

    if (!(cards = (card_state *) g_malloc(sizeof(card_state) * 32)))
        return NULL;

    for (ptr = g_list_first(gskat.cards); ptr; ptr = ptr->next)
    {
        tmp = (card *) ptr->data;
        cstate = &cards[i++];

        cstate->draw = tmp->draw;
        cstate->draw_face = tmp->draw_face;
        cstate->id = tmp->rank + tmp->suit;
        cstate->status = (gint) tmp->status;
    }

    return cards;
}

/**
 * @brief Save the current game states into a given output file
 *
 * @param filename  filename to write the states into
 *
 * @return TRUE on success, otherwise FALSE
 */
gboolean save_state_to_file(const gchar *filename)
{
    FILE *output;
    global_state *state;
    card_state *cstates;

    /* open file handle for writing */
    if (!(output = g_fopen(filename, "wb")))
    {
        DPRINT((_("Error on opening file '%s' for writing.\n"), filename));
        return FALSE;
    }

    /* try to determine global states */
    if (!(state = get_global_state()))
    {
        DPRINT((_("Could not determine current game state.\n")));

        fclose(output);
        return FALSE;
    }

    /* write global state into file buffer */
    if (fwrite(state, sizeof(global_state), 1, output) != 1)
    {
        DPRINT((_("Error on writing game state to file '%s'\n"), filename));

        fclose(output);
        g_free(state);
        return FALSE;
    }

    /* try to determine card states */
    if (!(cstates = get_card_states()))
    {
        DPRINT((_("Could not determine card states.\n")));

        fclose(output);
        g_free(state);
        return FALSE;
    }

    /* write card states into file buffer */
    if (fwrite(cstates, sizeof(card_state), 32, output) != 32)
    {
        DPRINT((_("Error on writing card states to file '%s'\n"), filename));

        fclose(output);
        g_free(state);
        g_free(cstates);
        return FALSE;

    }

    DPRINT((_("Wrote game state to file '%s'\n"), filename));

    fclose(output);
    g_free(state);
    g_free(cstates);
    return TRUE;
}

/**
 * @brief Read the game states saved inside a given file
 *
 * @param filename  filename of the state file to read/parse
 *
 * @return TRUE on success, otherwise FALSE
 */
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

    DPRINT(("%d\n%d\n%d\n", state->cplayer, state->num_stich, state->trump));

    fclose(input);
    g_free(state);
    return TRUE;
}

/* vim:set et sw=4 sts=4 tw=80: */
