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
#include "utils.h"
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

        cstate->draw      = tmp->draw;
        cstate->draw_face = tmp->draw_face;
        cstate->suit      = tmp->suit;
        cstate->rank      = tmp->rank;
        cstate->status    = (gint) tmp->status;
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
    gint i, num_cards;
    gint *card_ids;
    FILE *output;
    card *ptr;
    global_state *state;
    card_state *cstates;

    /* open file handle for writing (in binary mode) */
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

    /* write id's of all played cards into file buffer (in sequence) */
    num_cards = (gskat.stich - 1) * 3;

    if (num_cards && gskat.played)
    {
        card_ids = (gint *) g_malloc(sizeof(gint) * num_cards);

        for (i=0; i<num_cards; ++i)
        {
            ptr = g_list_nth_data(gskat.played, i);
            card_ids[i] = ptr->suit + ptr->rank;
        }

        if (fwrite(card_ids, sizeof(gint), num_cards, output) != num_cards)
        {
            DPRINT((_("Error on writing trick states to file '%s'\n"), filename));

            fclose(output);
            g_free(state);
            g_free(cstates);
            g_free(card_ids);
            return FALSE;
        }
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
    gint i = 0, num_cards = 0;
    gint *played_cards;
    FILE *input;
    global_state *state;
    card_state *cstates;

    /* allocate needed state structures */
    if (!(state = g_malloc(sizeof(global_state))))
        return FALSE;

    if (!(cstates = g_malloc(sizeof(card_state) * 32)))
        return FALSE;

    /* open file handle for reading (in binary mode) */
    if (!(input = g_fopen(filename, "rb")))
    {
        DPRINT((_("Error on opening file '%s' for reading.\n"), filename));

        g_free(state);
        g_free(cstates);
        return FALSE;
    }

    /* read global states */
    if (fread(state, sizeof(global_state), 1, input) != 1)
    {
        DPRINT((_("Error on reading game state from file '%s'\n"), filename));

        fclose(input);
        g_free(state);
        g_free(cstates);
        return FALSE;
    }

    /* read card states */
    if (fread(cstates, sizeof(card_state), 32, input) != 32)
    {
        DPRINT((_("Error on reading card states from file '%s'\n"), filename));

        fclose(input);
        g_free(state);
        g_free(cstates);
        return FALSE;
    }

    /* get number of played cards */
    num_cards = (state->num_stich - 1) * 3;
    played_cards = (gint *) g_malloc(sizeof(gint) * num_cards);

    if (num_cards)
    {
        if (fread(played_cards, sizeof(gint), num_cards, input) != num_cards)
        {
            DPRINT((_("Error on reading trick states from file '%s'\n"),
                        filename));

            fclose(input);
            g_free(state);
            g_free(cstates);
            g_free(played_cards);
            return FALSE;
        }

        DPRINT(("PLAYED_CARDS: (%d)\n", num_cards));
        for (i=0; i<num_cards; ++i)
        {
            DPRINT(("%d\n", played_cards[i]));
        }
    }

    DPRINT(("cur_player: %d\n", state->cplayer));
    DPRINT(("trick: %d\n", state->num_stich));
    DPRINT(("trump: %d\n", state->trump));
    DPRINT(("re_player: %d\n", state->re_player));
    DPRINT(("hand: %d\n", state->hand));
    DPRINT(("null: %d\n", state->null));

    for (i=0; i<32; ++i)
        DPRINT(("card: %s %s\tstatus: %d\n",
                    suit_name(cstates[i].suit),
                    rank_name(cstates[i].rank),
                    cstates[i].status));

    DPRINT((_("Successfully read game state from file '%s'\n"), filename));

    fclose(input);
    g_free(state);
    g_free(cstates);
    if (played_cards)
        g_free(played_cards);

    return TRUE;
}

/* vim:set et sw=4 sts=4 tw=80: */
