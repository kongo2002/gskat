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

    if (!(state = (global_state *) g_malloc(sizeof(global_state))))
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
        pstate->num_cards = g_list_length(cur_player->cards);
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
 * @todo This function needs some refactoring I think.
 *
 * @return TRUE on success, otherwise FALSE
 */
gboolean save_state_to_file(const gchar *filename)
{
    gint i, j, len, num_cards;
    gint *cards, *card_ids;
    FILE *output;
    card *ptr;
    player *pptr;
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

    /* write players' cards into file buffer */
    for (i=0; i<3; ++i)
    {
        pptr = gskat.players[i];
        len = g_list_length(pptr->cards);

        if (len)
        {
            cards = (gint *) g_malloc(sizeof(gint) * len);

            for (j=0; j<len; ++j)
            {
                ptr = g_list_nth_data(pptr->cards, j);
                cards[j] = ptr->rank + ptr->suit;
            }
        }
        else
        {
            len = 1;
            cards = (gint *) g_malloc(sizeof(gint));
            *cards = 0;
        }

        if (fwrite(cards, sizeof(gint), len, output) != len)
            return FALSE;

        g_free(cards);
    }

    DPRINT((_("Wrote game state to file '%s'\n"), filename));

    fclose(output);
    g_free(state);
    g_free(cstates);
    return TRUE;
}

global_state *read_global_state(FILE *input)
{
    global_state *state;

    if (!(state = (global_state *) g_malloc(sizeof(global_state))))
        return NULL;

    if (fread(state, sizeof(global_state), 1, input) != 1)
    {
        DPRINT((_("Error on reading game state.\n")));

        g_free(state);
        return NULL;
    }

    DPRINT(("cur_player: %d\n", state->cplayer));
    DPRINT(("trick: %d\n", state->num_stich));
    DPRINT(("trump: %d\n", state->trump));
    DPRINT(("re_player: %d\n", state->re_player));
    DPRINT(("hand: %d\n", state->hand));
    DPRINT(("null: %d\n", state->null));

    return state;
}

card_state *read_card_states(FILE *input)
{
    gint i;
    card_state *state;

    if (!(state = (card_state *) g_malloc(sizeof(card_state) * 32)))
        return NULL;

    if (fread(state, sizeof(card_state), 32, input) != 32)
    {
        DPRINT((_("Error on reading card states from file.\n")));

        g_free(state);
        return NULL;
    }

    for (i=0; i<32; ++i)
        DPRINT(("card: %s %s\tstatus: %d\n",
                    suit_name(state[i].suit),
                    rank_name(state[i].rank),
                    state[i].status));

    return state;
}

gint *read_played_cards_state(FILE *input, gint num_cards)
{
    gint i;
    gint *played_cards = NULL;

    if (num_cards)
    {
        played_cards = (gint *) g_malloc(sizeof(gint) * num_cards);

        if (fread(played_cards, sizeof(gint), num_cards, input) != num_cards)
        {
            DPRINT((_("Error on reading trick states.\n")));

            g_free(played_cards);
            return NULL;
        }

        DPRINT(("PLAYED_CARDS: (%d)\n", num_cards));
        for (i=0; i<num_cards; ++i)
            DPRINT(("%d\n", played_cards[i]));
    }

    return played_cards;
}

gboolean read_players_cards_state(FILE *input, state_group *sg,
        global_state *gs)
{
    gint i, j, len;
    gint *cards;

    /* initialize player cards integer array */
    sg->pcards = (gint **) g_malloc(sizeof(gint *) * 3);

    for (i=0; i<3; ++i)
        sg->pcards[i] = NULL;

    /* get players' cards */
    for (i=0; i<3; ++i)
    {
        len = gs->pstates[i].num_cards;

        if (!len)
            len = 1;

        cards = (gint *) g_malloc(sizeof(gint) * len);

        if (fread(cards, sizeof(gint), len, input) != len)
        {
            DPRINT((_("Error on reading players' cards state.\n")));

            for (j=0; j<3; ++j)
                if (sg->pcards[j])
                    g_free(sg->pcards[j]);
            g_free(cards);

            return FALSE;
        }

        DPRINT((_("Player %d cards: "), i));

        for (j=0; j<len; ++j)
        {
            if (j)
                DPRINT((", "));
            DPRINT(("%d", cards[j]));
        }
        DPRINT(("\n"));

        sg->pcards[i] = cards;
    }

    return TRUE;
}

/**
 * @brief Read the game states saved inside a given file
 *
 * @param filename  filename of the state file to read/parse
 *
 * @todo This function needs some refactoring I think.
 *
 * @return TRUE on success, otherwise FALSE
 */
gboolean read_state_from_file(const gchar *filename)
{
    gint num_cards;
    gint *played_cards;
    FILE *input;
    global_state *state;
    card_state *cstates;
    state_group *sg;

    /* allocate memory for state group structure */
    sg = (state_group *) g_malloc(sizeof(state_group));

    /* open file handle for reading (in binary mode) */
    if (!(input = g_fopen(filename, "rb")))
    {
        DPRINT((_("Error on opening file '%s' for reading.\n"), filename));

        g_free(sg);
        return FALSE;
    }

    /* read global states */
    if ((state = read_global_state(input)) == NULL)
        goto read_state_error;

    /* read card states */
    if ((cstates = read_card_states(input)) == NULL)
        goto read_state_error;

    /* get number of played cards */
    num_cards = (state->num_stich - 1) * 3;

    /* read played cards */
    if ((played_cards = read_played_cards_state(input, num_cards)) == NULL)
        goto read_state_error;

    /* get players' cards */
    if (!read_players_cards_state(input, sg, state))
        goto read_state_error;

    DPRINT((_("Successfully read game state from file '%s'\n"), filename));

    sg->gs = state;
    sg->cs = cstates;
    sg->pc = played_cards;
    sg->num_played = num_cards;

    /* apply_states(sg); */

    fclose(input);

    return TRUE;

read_state_error:

    DPRINT((_("Failed to read game state from file '%s'\n"), filename));

    fclose(input);
    g_free(sg);

    return FALSE;
}

/**
 * @brief Apply all read states to the current game state
 *
 * @param sg  state group structure
 */
void apply_states(state_group *sg)
{
    gint i, j;
    card *crd;
    card_state *cs;
    player *pptr;
    player_state *pstate;

    /* reset current game states */
    reset_game();

    /* set global game properties */
    gskat.stich   = sg->gs->num_stich;
    gskat.cplayer = sg->gs->cplayer;
    gskat.trump   = sg->gs->trump;
    gskat.hand    = sg->gs->hand;
    gskat.null    = sg->gs->null;
    gskat.re      = gskat.players[sg->gs->re_player];

    /* set player values */
    for (i=0; i<3; ++i)
    {
        pptr = gskat.players[i];
        pstate = &sg->gs->pstates[i];

        pptr->re         = pstate->re;
        pptr->gereizt    = pstate->gereizt;
        pptr->points     = pstate->points;
        pptr->sum_points = pstate->sum_points;
    }

    /* set all card values accordingly */
    for (i=0; i<32; ++i)
    {
        cs = &sg->cs[i];
        crd = get_card_ptr(cs->suit, cs->rank);

        crd->owner     = cs->owner;
        crd->status    = cs->status;
        crd->draw      = cs->draw;
        crd->draw_face = cs->draw_face;
    }

    /* TODO: we have to fill the card lists accordingly here, like
     * 'skat', 'table', 'played' and 'stiche' */

    /* populate played cards list */
    for (i=0; i<sg->num_played; ++i)
        gskat.played = g_list_append(gskat.played, get_card_by_id(sg->pc[i]));

    /* populate stiche array */
    for (i=0; i<sg->gs->num_stich-1; ++i)
    {
        gskat.stiche[i] = (card **) g_malloc(sizeof(card *) * 3);

        for (j=0; j<3; ++j)
            gskat.stiche[i][j] = get_card_by_id(sg->pc[i*3+j]);
    }

    /* trigger continuation of the game */
    gskat.state = PLAYING;
}

/* vim:set et sw=4 sts=4 tw=80: */
