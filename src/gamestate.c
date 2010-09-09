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
    card *ptr;
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
    state->cplayer    = gskat.cplayer;
    state->num_stich  = gskat.stich;
    state->trump      = gskat.trump;
    state->re_player  = (gskat.re) ? gskat.re->id : -1;
    state->null       = gskat.null;
    state->hand       = gskat.hand;
    state->num_played = g_list_length(gskat.played);

    /* add cards in skat */
    for (i=0; i<2; ++i)
    {
        ptr = g_list_nth_data(gskat.skat, i);
        state->skat[i] = ptr->rank + ptr->suit;
    }

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
 * @brief Write global states into file buffer
 *
 * @param output  output file stream
 *
 * @return TRUE on success, otherwise FALSE
 */
gboolean save_global_state(FILE *output)
{
    global_state *state;

    /* try to determine global states */
    if (!(state = get_global_state()))
    {
        DPRINT((_("Could not determine current game state.\n")));
        return FALSE;
    }

    /* write global state into file buffer */
    if (fwrite(state, sizeof(global_state), 1, output) != 1)
    {
        DPRINT((_("Error on writing game state.\n")));

        g_free(state);
        return FALSE;
    }

    g_free(state);
    return TRUE;
}

/**
 * @brief Write card states of all 32 game cards into file buffer
 *
 * @param output  output file stream
 *
 * @return TRUE on success, otherwise FALSE
 */
gboolean save_card_states(FILE *output)
{
    card_state *state;

    /* try to determine card states */
    if (!(state = get_card_states()))
    {
        DPRINT((_("Could not determine card states.\n")));
        return FALSE;
    }

    /* write card states into file buffer */
    if (fwrite(state, sizeof(card_state), 32, output) != 32)
    {
        DPRINT((_("Error on writing card states.\n")));

        g_free(state);
        return FALSE;
    }

    g_free(state);
    return TRUE;
}

/**
 * @brief Write id's of all played cards into file buffer
 *
 * @param output  output file stream
 *
 * @return TRUE on success, otherwise FALSE
 */
gboolean save_played_card_states(FILE *output)
{
    gint i, num_cards;
    gint *card_ids;
    card *ptr;

    num_cards = g_list_length(gskat.played);

    if (num_cards)
    {
        card_ids = (gint *) g_malloc(sizeof(gint) * num_cards);

        for (i=0; i<num_cards; ++i)
        {
            ptr = g_list_nth_data(gskat.played, i);
            card_ids[i] = ptr->suit + ptr->rank;
        }

        if (fwrite(card_ids, sizeof(gint), num_cards, output) != num_cards)
        {
            DPRINT((_("Error on writing trick states.\n")));

            g_free(card_ids);
            return FALSE;
        }
    }

    g_free(card_ids);
    return TRUE;
}

/**
 * @brief Write players' cards into file buffer
 *
 * @param output  output file stream
 *
 * @return TRUE on success, otherwise FALSE
 */
gboolean save_players_cards_state(FILE *output)
{
    gint i, j, len;
    gint *cards;
    card *ptr;
    player *pptr;

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
        {
            DPRINT((_("Failed on writing players' cards state.\n")));

            g_free(cards);
            return FALSE;
        }

        g_free(cards);
    }

    return TRUE;
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

    /* open file handle for writing (in binary mode) */
    if (!(output = g_fopen(filename, "wb")))
    {
        DPRINT((_("Error on opening file '%s' for writing.\n"), filename));
        return FALSE;
    }

    if (!save_global_state(output))
        goto save_state_error;

    if (!save_card_states(output))
        goto save_state_error;

    if (!save_played_card_states(output))
        goto save_state_error;

    if (!save_players_cards_state(output))
        goto save_state_error;

    DPRINT((_("Successfully wrote game state to file '%s'\n"), filename));

    fclose(output);
    return TRUE;

save_state_error:

    DPRINT((_("Failed to write game state to file '%s'\n"), filename));

    fclose(output);
    return FALSE;
}

/**
 * @brief Read global states from input file
 *
 * @param input  input file stream
 *
 * @return new global_state structure or NULL on error
 */
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
    DPRINT(("num_played: %d\n", state->num_played));
    DPRINT(("skat: %d\t%d\n", state->skat[0], state->skat[1]));

    return state;
}

/**
 * @brief Read card states from all 32 game cards
 *
 * @param input  input file stream
 *
 * @return new card_state array or NULL on error
 */
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

/**
 * @brief Read all played cards from input file
 *
 * @param input      input file stream
 * @param num_cards  number of played cards
 *
 * @return integer array containing played card ids or NULL on error
 */
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

/**
 * @brief Read players' cards from input file
 *
 * @param input  input file stream
 * @param sg     state group structure
 * @param gs     global states
 *
 * @return TRUE on success, otherwise FALSE
 */
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
            g_free(sg->pcards);

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
    num_cards = state->num_played;

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
     * TODO: 'skat', 'table', 'played' and 'stiche' */

    /* populate played cards list */
    for (i=0; i<sg->gs->num_played; ++i)
        gskat.played = g_list_append(gskat.played, get_card_by_id(sg->pc[i]));

    /* fill stiche array */
    /* TODO: we have to add remaining cards of the last trick
     * TODO: to the stiche array as well*/
    for (i=0; i<sg->gs->num_stich-1; ++i)
    {
        gskat.stiche[i] = (card **) g_malloc(sizeof(card *) * 3);

        for (j=0; j<3; ++j)
            gskat.stiche[i][j] = get_card_by_id(sg->pc[i*3+j]);
    }

    /* populate players' cards list */
    for (i=0; i<3; ++i)
    {
        pptr = gskat.players[i];

        for (j=0; j<sg->gs->pstates[3].num_cards; ++j)
        {
            pptr->cards = g_list_append(pptr->cards,
                    get_card_by_id(sg->pcards[i][j]));
        }
    }

    /* populate skat list */
    for (i=0; i<2; ++i)
        gskat.skat = g_list_append(gskat.skat, get_card_by_id(sg->gs->skat[i]));

    /* trigger continuation of the game */
    gskat.state = PLAYING;
}

/* vim:set et sw=4 sts=4 tw=80: */
