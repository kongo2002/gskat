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
#include "ai.h"
#include "common.h"
#include "null.h"
#include "utils.h"

/**
 * null_select_card:
 * @player:   Player to choose the next card to play for
 * @list:     A #GList with all possible cards to choose from
 * @position: Player's position on the table
 *
 * Null game AI function to determine the next card to play according to the
 * position and the team the player is in
 *
 * Returns: a #card to play or %NULL if no suitable #card could be found
 */
card *null_select_card(player *player, GList *list, gint position)
{
    card *card = NULL;

    /* re player */
    if (player->re)
    {
        /* player is first to play */
        if (position == 0)
        {
            card = null_re_kommt_raus(player, list);
        }
        /* player is in the middle */
        else if (position == 1)
        {
            /* do some */
        }
        /* player is the last to play */
        else
        {
            card = null_re_hinten(player, list);
        }
    }
    /* kontra player */
    else
    {
        /* player is first to play */
        if (position == 0)
        {
            card = null_kontra_kommt_raus(player, list);
        }
        /* player is in the middle */
        else if (position == 1)
        {
            card = null_kontra_mitte(player, list);
        }
        /* player is the last to play */
        else
        {
            card = null_kontra_hinten(player, list);
        }
    }

    return card;
}

/**
 * null_re_kommt_raus:
 * @player: Player to choose a card to play for
 * @list:   A #GList with all possible cards to choose from
 *
 * Determine the strategy to play for the Re player
 * if he is the first to play and it's a null game.
 *
 * Returns: the selected card or %NULL
 */
card *null_re_kommt_raus(player *player, GList *list)
{
    card *card = NULL;

    gskat_msg(MT_DEBUG | MT_BUGREPORT, "%s: null_re_kommt_raus()\n",
            player->name);

    if ((card = null_aufspielen(player, list)))
        return card;

    return card;
}

/**
 * null_kontra_kommt_raus:
 * @player: Player to choose a card to play for
 * @list:   A #GList with all possible cards to choose from
 *
 * Determine the strategy to play for the Kontra player
 * if he is the first to play and it's a null game.
 *
 * Returns: the selected card or %NULL
 */
card *null_kontra_kommt_raus(player *player, GList *list)
{
    card *card = NULL;

    gskat_msg(MT_DEBUG | MT_BUGREPORT, "%s: null_kontra_kommt_raus()\n",
            player->name);

    if ((card = null_aufspielen(player, list)))
        return card;

    return NULL;
}

/**
 * null_kontra_mitte:
 * @player: Player to choose a card to play for
 * @list:   A #GList with all possible cards to choose from
 *
 * Determine the strategy to play for the Kontra player
 * if he is the second to play and it's a null game.
 *
 * Returns: the selected card or %NULL
 */
card *null_kontra_mitte(player *player, GList *list)
{
    card *card = NULL;

    gskat_msg(MT_DEBUG | MT_BUGREPORT, "%s: null_kontra_mitte()\n", player->name);

    if (muss_bedienen(player))
    {
        if ((card = niedrig_bedienen(player, list)))
            return card;
    }
    else
    {
        if ((card = null_hoch_abwerfen(player, list)))
            return card;
    }

    return card;
}

/**
 * null_kontra_hinten:
 * @player: Player to choose a card to play for
 * @list:   A #GList with all possible cards to choose from
 *
 * Determine the strategy to play for the Kontra player
 * if he is the last to play and it's a null game.
 *
 * Returns: the selected card or %NULL
 */
card *null_kontra_hinten(player *player, GList *list)
{
    card *tmp, *card = NULL;

    gskat_msg(MT_DEBUG | MT_BUGREPORT, "%s: null_kontra_hinten()\n",
            player->name);

    tmp = highest_on_table();

    if (tmp->owner == gskat.re->id)
    {
        if (muss_bedienen(player))
        {
            if ((card = drunter_bleiben(player, list)))
                return card;
        }
        else
        {
            if ((card = null_hoch_abwerfen(player, list)))
                return card;
        }
    }
    else
    {
        if ((card = null_hoch_abwerfen(player, list)))
            return card;
    }

    return NULL;
}

/**
 * null_re_hinten:
 * @player: Player to choose a card to play for
 * @list:   A #GList with all possible cards to choose from
 *
 * Determine the strategy to play for the Re player
 * if he is the last to play and it's a null game.
 *
 * Returns: the selected card or %NULL
 */
card *null_re_hinten(player *player, GList *list)
{
    card *card = NULL;

    gskat_msg(MT_DEBUG | MT_BUGREPORT, "%s: null_re_hinten()\n", player->name);

    if ((card = drunter_bleiben(player, list)))
        return card;

    return NULL;
}

/**
 * null_aufspielen:
 * @player: Player to choose a #card to play for
 * @list:   A #GList with all possible cards to choose from
 *
 * Play the lowest card available and the one of the shortest suit
 * if there are two or more card of the same (low) rank.
 *
 * Returns: the selected #card or %NULL
 */
card *null_aufspielen(player *player, GList *list)
{
    gint i, len, min = -1;
    GList *suit_l;
    card *ptr, *ret_card = NULL;

    gskat_msg(MT_DEBUG | MT_BUGREPORT,
            "%s: try null_aufspielen()\n", player->name);

    /* iterate through all four suits */
    for (i=0; i<4; ++i)
    {
        /* kontra player's should not play a suit that the
         * re player has already trumped */
        if (!player->re && hat_gestochen(gskat.re, SUITS[i]))
        {
            gskat_msg(MT_DEBUG | MT_BUGREPORT,
                    "Skip '%s' - Re has already trumped that one\n",
                    suit_name(SUITS[i]));

            continue;
        }

        suit_l = get_suit_list(list, SUITS[i]);

        if (suit_l)
        {
            /* get lowest card of the current suit */
            len = g_list_length(suit_l);
            ptr = g_list_nth_data(suit_l, len - 1);

            /* choose the lowest card available and the shortest suit
             * if there are two or more of the same rank */
            if (ret_card == NULL || !higher_rank(ptr, ret_card) ||
                    (ptr->rank == ret_card->rank && len < min))
            {
                ret_card = ptr;
                min = len;
            }

            g_list_free(suit_l);
        }
    }

    return ret_card;
}

/**
 * niedrig_bedienen:
 * @player: Player to choose a #card to play for
 * @list:   A #GList with all possible cards to choose from
 *
 * Select a the lowest card possible
 *
 * Returns: the selected #card or %NULL
 */
card *niedrig_bedienen(player *player, GList *list)
{
    gskat_msg(MT_DEBUG | MT_BUGREPORT,
            "%s: try niedrig_bedienen()\n", player->name);

    return g_list_nth_data(list, g_list_length(list) - 1);
}

/**
 * drunter_bleiben:
 * @player: Player to choose a #card to play for
 * @list:   A #GList with all possible cards to choose from
 *
 * Try to find a lower card than the highest on the table
 *
 * Returns: the selected #card or %NULL
 */
card *drunter_bleiben(player *player, GList *list)
{
    GList *ptr;
    card *tmp, *high = highest_on_table();

    gskat_msg(MT_DEBUG | MT_BUGREPORT,
            "%s: try drunter_bleiben()\n", player->name);

    for (ptr = g_list_first(list); ptr; ptr = ptr->next)
    {
        tmp = (card *) ptr->data;

        if (!is_greater(tmp, high))
            return tmp;
    }

    return NULL;
}

/**
 * null_hoch_abwerfen:
 * @player: Player to choose a #card to play for
 * @list:   A #GList with all possible cards to choose from
 *
 * Select a high card to throw away - if possible from a short
 * numbered suit.
 *
 * Returns: the selected #card or %NULL
 */
card *null_hoch_abwerfen(player *player, GList *list)
{
    GList *suit_l;
    card *ret_card = NULL;

    gskat_msg(MT_DEBUG | MT_BUGREPORT,
            "%s: try null_hoch_abwerfen()\n", player->name);

    /* if player has to serve the suit
     * simply play the highest available */
    if (muss_bedienen(player))
        return (card *) list->data;

    /* get shortest suit */
    suit_l = get_short_suit(list, 0);

    if (suit_l)
    {
        ret_card = (card *) suit_l->data;

        /* do not choose a card with no points (7, 8, 9) */
        if (!ret_card->points)
            ret_card = NULL;

        g_list_free(suit_l);
    }

    return ret_card;
}

/* vim:set et sw=4 sts=4 tw=80: */
