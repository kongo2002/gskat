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
#include "game.h"
#include "gamestate.h"

/**
 * get_global_state:
 *
 * Determine the current global game states like round, trick
 * and players' points
 *
 * Returns: (transfer full): new #global_state object
 */
global_state *get_global_state(void)
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
    state->forehand   = gskat.forehand;
    state->num_played = g_list_length(gskat.played);
    state->num_table  = g_list_length(gskat.table);

    /* add cards in skat */
    for (i=0; i<2; ++i)
    {
        ptr = g_list_nth_data(gskat.skat, i);
        state->skat[i] = ptr->rank + ptr->suit;
    }

    return state;
}

/**
 * get_card_states:
 *
 * Determine the current card states
 *
 * Returns: (transfer full): an array of 32 #card_state objects
 */
card_state *get_card_states(void)
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

        cstate->owner     = tmp->owner;
        cstate->draw      = tmp->draw;
        cstate->draw_face = tmp->draw_face;
        cstate->suit      = tmp->suit;
        cstate->rank      = tmp->rank;
        cstate->status    = (gint) tmp->status;
    }

    return cards;
}

/**
 * save_global_state:
 * @output:  output file stream
 *
 * Write global states into file buffer
 *
 * Returns: %TRUE on success, otherwise %FALSE
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
 * save_card_states:
 * @output:  output file stream
 *
 * Write card states of all 32 game cards into file buffer
 *
 * Returns: %TRUE on success, otherwise %FALSE
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
 * save_played_card_states:
 * @output:  output file stream
 *
 * Write id's of all played cards into file buffer
 *
 * Returns: %TRUE on success, otherwise %FALSE
 */
gboolean save_played_card_states(FILE *output)
{
    guint i, num_cards;
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

        g_free(card_ids);
    }

    return TRUE;
}

/**
 * save_players_cards_state:
 * @output:  output file stream
 *
 * Write players' cards into file buffer
 *
 * Returns: %TRUE on success, otherwise %FALSE
 */
gboolean save_players_cards_state(FILE *output)
{
    guint i, j, len;
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

            if (fwrite(cards, sizeof(gint), len, output) != len)
            {
                DPRINT((_("Failed on writing players' cards state.\n")));

                g_free(cards);
                return FALSE;
            }

            g_free(cards);
        }
    }

    return TRUE;
}

/**
 * save_table_state:
 * @output:  output file stream
 *
 * Write table cards into file buffer
 *
 * Returns: %TRUE on success, otherwise %FALSE
 */
gboolean save_table_state(FILE *output)
{
    guint i, len;
    gint *table;
    card *ptr;

    len = g_list_length(gskat.table);

    if (len)
    {
        table = (gint *) g_malloc(sizeof(gint) * len);

        for (i=0; i<len; ++i)
        {
            ptr = g_list_nth_data(gskat.table, i);
            table[i] = ptr->rank + ptr->suit;
        }

        if (fwrite(table, sizeof(gint), len, output) != len)
        {
            DPRINT((_("Failed on writing table cards state.\n")));

            g_free(table);
            return FALSE;
        }

        g_free(table);
    }

    return TRUE;
}

/**
 * save_state_to_file:
 * @filename:  filename to write the states into
 *
 * Save the current game states into a given output file
 *
 * Returns: %TRUE on success, otherwise %FALSE
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

    if (!save_table_state(output))
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
 * read_global_state:
 * @input:  input file stream
 *
 * Read global states from input file
 *
 * Returns: (transfer full): new #global_state object or %NULL on error
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
    DPRINT(("forehand: %d\n", state->forehand));
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
 * read_card_states:
 * @input:  input file stream
 *
 * Read card states from all 32 game cards
 *
 * Returns: new #card_state array or %NULL on error
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
 * read_played_cards_state:
 * @input:      input file stream
 * @sg:         state group structure
 * @num_cards:  number of played cards
 *
 * Read all played cards from input file
 *
 * Returns: integer array containing played card ids or %NULL on error
 */
gboolean read_played_cards_state(FILE *input, state_group *sg, guint num_cards)
{
    guint i;
    sg->pc = NULL;

    if (num_cards)
    {
        sg->pc = (gint *) g_malloc(sizeof(gint) * num_cards);

        if (fread(sg->pc, sizeof(gint), num_cards, input) != num_cards)
        {
            DPRINT((_("Error on reading trick states.\n")));

            g_free(sg->pc);
            sg->pc = NULL;

            return FALSE;
        }

        DPRINT(("PLAYED_CARDS (%d):\n", num_cards));
        for (i=0; i<num_cards; ++i)
            DPRINT(("%d\n", sg->pc[i]));
    }

    return TRUE;
}

/**
 * read_players_cards_state:
 * @input:  input file stream
 * @sg:     #state_group object
 * @gs:     #global_state object
 *
 * Read players' cards from input file
 *
 * Returns: %TRUE on success, otherwise %FALSE
 */
gboolean read_players_cards_state(FILE *input, state_group *sg,
        global_state *gs)
{
    guint i, j, len;
    gint *cards;

    /* initialize player cards integer array */
    sg->pcards = (gint **) g_malloc(sizeof(gint *) * 3);

    for (i=0; i<3; ++i)
        sg->pcards[i] = NULL;

    /* get players' cards */
    for (i=0; i<3; ++i)
    {
        len = gs->pstates[i].num_cards;

        if (len)
        {
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
        else
        {
            DPRINT((_("Player %d has no cards anymore.\n"), i));

            sg->pcards[i] = NULL;
        }
    }

    return TRUE;
}

/**
 * read_table_state:
 * @input:      input file stream
 * @sg:         state group structure
 * @num_table:  number of cards on the table
 *
 * Read table cards from input file
 *
 * Returns: %TRUE on success, otherwise %FALSE
 */
gboolean read_table_state(FILE *input, state_group *sg, guint num_table)
{
    guint i;

    if (num_table)
    {
        sg->table = (gint *) g_malloc(sizeof(gint) * num_table);

        if (fread(sg->table, sizeof(gint), num_table, input) != num_table)
        {
            DPRINT((_("Error on reading table cards state.\n")));

            g_free(sg->table);
            sg->table = NULL;

            return FALSE;
        }

        DPRINT((_("Cards on the table: ")));

        for (i=0; i<num_table; ++i)
        {
            if (i)
                DPRINT((", "));
            DPRINT(("%d", sg->table[i]));
        }
        DPRINT(("\n"));
    }
    else
    {
        DPRINT((_("There are no cards on the table.\n")));

        sg->table = NULL;
    }

    return TRUE;
}

/**
 * read_state_from_file:
 * @filename:  filename of the state file to read/parse
 *
 * Read the game states saved inside a given file
 *
 * Returns: %TRUE on success, otherwise %FALSE
 */
gboolean read_state_from_file(const gchar *filename)
{
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

    /* read played cards */
    if (!read_played_cards_state(input, sg, state->num_played))
        goto read_state_error;

    /* get players' cards */
    if (!read_players_cards_state(input, sg, state))
        goto read_state_error;

    /* get cards on the table */
    if (!read_table_state(input, sg, state->num_table))
        goto read_state_error;

    DPRINT((_("Successfully read game state from file '%s'\n"), filename));

    sg->gs = state;
    sg->cs = cstates;

    fclose(input);

    apply_states(sg);

    return TRUE;

read_state_error:

    DPRINT((_("Failed to read game state from file '%s'\n"), filename));

    fclose(input);
    g_free(sg);

    return FALSE;
}

/**
 * apply_states:
 * @sg:  #state_group object
 *
 * Apply all read states to the current game state
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
    gskat.stich    = sg->gs->num_stich;
    gskat.cplayer  = sg->gs->cplayer;
    gskat.forehand = sg->gs->forehand;
    gskat.trump    = sg->gs->trump;
    gskat.hand     = sg->gs->hand;
    gskat.null     = sg->gs->null;
    gskat.re       = gskat.players[sg->gs->re_player];

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

    /* populate played cards list */
    for (i=0; i<sg->gs->num_played; ++i)
        gskat.played = g_list_append(gskat.played, get_card_by_id(sg->pc[i]));

    /* fill stiche array */
    for (i=0; i<sg->gs->num_stich-1; ++i)
    {
        gskat.stiche[i] = (card **) g_malloc(sizeof(card *) * 3);

        for (j=0; j<3; ++j)
            gskat.stiche[i][j] = get_card_by_id(sg->pc[i*3+j]);
    }

    /* add cards on the table to the stiche array */
    if (sg->gs->num_table)
    {
        i = sg->gs->num_stich - 1;
        gskat.stiche[i] = (card **) g_malloc(sizeof(card *) * 3);

        for (j=0; j<3; ++j)
        {
            if (j < sg->gs->num_table)
                gskat.stiche[i][j] = get_card_by_id(sg->table[j]);
            else
                gskat.stiche[i][j] = NULL;
        }
    }

    /* populate players' cards list */
    for (i=0; i<3; ++i)
    {
        pptr = gskat.players[i];

        for (j=0; j<sg->gs->pstates[i].num_cards; ++j)
        {
            pptr->cards = g_list_append(pptr->cards,
                    get_card_by_id(sg->pcards[i][j]));
        }
    }

    /* populate skat list */
    for (i=0; i<2; ++i)
        gskat.skat = g_list_append(gskat.skat, get_card_by_id(sg->gs->skat[i]));

    /* populate table cards list */
    for (i=0; i<sg->gs->num_table; ++i)
        gskat.table = g_list_append(gskat.table, get_card_by_id(sg->table[i]));

    /* free states memory */
    g_free(sg->gs);
    g_free(sg->cs);

    if (sg->pc)
        g_free(sg->pc);

    if (sg->table)
        g_free(sg->table);

    for (i=0; i<3; ++i)
    {
        if (sg->pcards[i])
            g_free(sg->pcards[i]);
    }

    g_free(sg->pcards);
    g_free(sg);
}

/* vim:set et sw=4 sts=4 tw=80: */
