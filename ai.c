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
#include "game.h"
#include "utils.h"

/**
 * @brief Main AI function to determine the next card to play according to the
 * position and the team the player is in
 *
 * @param player  player to choose the next card to play for
 * @param list    possible cards to choose from
 *
 * @return the card to play next
 */
card *ai_select_card(player *player, GList *list)
{
    gint selection;
    gint position;
    card *card = NULL;

    /* return last card */
    if (g_list_length(list) == 1)
    {
        DPRINT(("%s: only 1 card possible\n", player->name));
        return g_list_nth_data(list, 0);
    }

    /* determine position on table */
    position = (gskat.table) ? g_list_length(gskat.table) : 0;

    /* re player */
    if (player->re)
    {
        /* player is first to play */
        if (position == 0)
        {
            card = ai_re_kommt_raus(player, list);
        }
        /* player is in the middle */
        else if (position == 1)
        {
            card = ai_re_mitte(player, list);
        }
        /* player is the last to play */
        else
        {
            card = ai_re_hinten(player, list);
        }
    }
    /* kontra player */
    else
    {
        /* player is first to play */
        if (position == 0)
        {
            card = ai_kontra_kommt_raus(player, list);
        }
        /* player is in the middle */
        else if (position == 1)
        {
            card = ai_kontra_mitte(player, list);
        }
        /* player is the last to play */
        else
        {
            card = ai_kontra_hinten(player, list);
        }
    }

    /* temporary fallback as long as not
     * all strategies are implemented yet */
    if (card == NULL)
    {
        DPRINT(("%s: random card\n", player->name));

        selection = rand() % g_list_length(list);
        return g_list_nth_data(list, selection);
    }

    return card;
}

/**
 * @brief try to select an ace from a short numbered suit
 *
 * @param player  player to choose a card to play for
 * @param list    possible cards to choose from
 *
 * @return the selected card or NULL
 */
card *kurz_fehl_ass(player *player, GList *list)
{
    gint i, len, min = -1;
    GList *ptr = NULL;
    card *card = NULL, *ret = NULL;

    DPRINT(("%s: try kurz_fehl_ass()\n", player->name));

    for (i=0; i<4; ++i)
    {
        if (SUITS[i] != gskat.trump)
        {
            if ((ptr = get_suit_list(list, SUITS[i])))
            {
                card = ptr->data;
                len = g_list_length(ptr);

                /* TODO: select suit of which the amount of played
                 * cards is minimum */
                /* TODO: check if someone already took the trick */
                if (card->rank == ASS && (min == -1 || len < min))
                {
                    min = len;
                    ret = card;
                }

                g_list_free(ptr);
            }
        }
    }

    return ret;
}

/**
 * @brief try to select a minimal card to get the trick on the table
 *
 * @param player  player to choose a card to play for
 * @param list    possible cards to choose from
 *
 * @return the selected card or NULL
 */
card *knapp_trumpfen(player *player, GList *list)
{
    GList *ptr = NULL;
    card *ret = NULL, *card = NULL, *high = highest_on_table();

    DPRINT(("%s: try knapp_trumpfen()\n", player->name));

    /* play ace if first trick of suit */
    if (!is_trump(high))
    {
        if ((ptr = get_suit_list(gskat.played, high->suit)))
        {
            if (g_list_length(ptr) < 2)
            {
                card = g_list_nth_data(list, 0);

                if (card->rank == ASS &&
                        is_greater(card, high, gskat.trump, gskat.null))
                {
                    g_list_free(ptr);
                    return card;
                }
            }

            g_list_free(ptr);
            ptr = NULL;
            card = NULL;
        }
    }

    for (ptr = g_list_first(list); ptr; ptr = ptr->next)
    {
        card = ptr->data;

        if (is_greater(card, high, gskat.trump, gskat.null))
            ret = card;
    }
    return ret;
}

/**
 * @brief Determine the strategy to play for the Re player
 * if he is the first to play
 *
 * @param player  player to choose a card to play for
 * @param list    possible cards to choose from
 *
 * @return the selected card or NULL
 */
card *ai_re_kommt_raus(player *player, GList *list)
{
    card *card = NULL;

    if ((card = highest_fehl(player, list)))
        return card;

    if ((card = kurz_fehl_ass(player, list)))
        return card;

    if ((card = trumpf_spitzen(player, list)))
        return card;

    if ((card = truempfe_ziehen(player, list)))
        return card;

    return card;
}

/**
 * @brief Determine the strategy to play for the Kontra player
 * if he is the first to play
 *
 * @param player  player to choose a card to play for
 * @param list    possible cards to choose from
 *
 * @return the selected card or NULL
 */
card *ai_kontra_kommt_raus(player *player, GList *list)
{
    card *card = NULL;

    if ((card = highest_fehl(player, list)))
        return card;

    if ((card = kurz_fehl_ass(player, list)))
        return card;

    /* partner sits at the end */
    if (gskat.players[(player->id+1)%3]->re)
    {
        card = kurz_aufspielen(player, list);
    }
    /* re player sits at the end */
    else
    {
        card = lang_aufspielen(player, list);
    }

    return card;
}

/**
 * @brief Determine the strategy to play for the Re player
 * if he is the second to play
 *
 * @param player  player to choose a card to play for
 * @param list    possible cards to choose from
 *
 * @return the selected card or NULL
 */
card *ai_re_mitte(player *player, GList *list)
{
    card *card = NULL, *first = NULL, *sel = NULL;

    /* played card */
    first = g_list_nth_data(gskat.table, 0);

    /* first of possible cards */
    sel = g_list_nth_data(list, 0);

    /* do not play a 10 when the ace is not played yet */
    if (!is_trump(first) &&
            muss_bedienen(player) &&
            num_of_suit(gskat.played, first->suit) == 1 &&
            sel->rank == 10)
    {
        DPRINT(("%s: 10 skipped - ace not played yet\n", player->name));
        card = abwerfen(player, list);
    }
    else
    {
        if ((card = knapp_trumpfen(player, list)))
            return card;
        else
            card = abwerfen(player, list);
    }

    return card;
}

/**
 * @brief Determine the strategy to play for the Kontra player
 * if he is the second to play
 *
 * @param player  player to choose a card to play for
 * @param list    possible cards to choose from
 *
 * @return the selected card or NULL
 */
card *ai_kontra_mitte(player *player, GList *list)
{
    card *tmp = NULL;
    card *card = NULL;

    tmp = g_list_nth_data(gskat.table, 0);

    if (!gskat.players[tmp->owner]->re)
    {
        if (kontra_stich_sicher(player))
            card = ai_kontra_schmieren(player, list);
    }

    if (!card)
    {
        if ((card = knapp_trumpfen(player, list)))
            return card;
        else
            card = abwerfen(player, list);
    }

    return card;
}

/**
 * @brief Determine the strategy to play for the Kontra player
 * if he is the last to play
 *
 * @param player  player to choose a card to play for
 * @param list    possible cards to choose from
 *
 * @return the selected card or NULL
 */
card *ai_kontra_hinten(player *player, GList *list)
{
    card *card = NULL;

    if (kontra_stich_sicher(player))
        return ai_kontra_schmieren(player, list);
    /* trump only if enough points on the table */
    else if (muss_bedienen(player) || punkte_auf_tisch() > 2)
        card = knapp_trumpfen(player, list);

    if (!card)
        card = abwerfen(player, list);

    return card;
}

/**
 * @brief Determine the strategy to play for the Re player
 * if he is the last to play
 *
 * @param player  player to choose a card to play for
 * @param list    possible cards to choose from
 *
 * @return the selected card or NULL
 */
card *ai_re_hinten(player *player, GList *list)
{
    card *card = NULL;

    if ((card = knapp_trumpfen(player, list)))
        return card;
    else
        card = abwerfen(player, list);

    return card;
}

/**
 * @brief Try to select one random high trump card
 *
 * @param player  player to choose a card to play for
 * @param list    possible cards to choose from
 *
 * @return the selected card or NULL
 */
card *trumpf_spitzen(player *player, GList *list)
{
    GList *trump = get_trump_list(list);
    gint spitzen = 0;
    card *card = NULL;

    DPRINT(("%s: try trumpf_spitzen()\n", player->name));

    if (trump)
    {
        if (!gskat.null)
        {
            spitzen = len_spitzen(player, trump, gskat.trump);

            if (spitzen)
                card = g_list_nth_data(trump, rand() % spitzen);
        }

        g_list_free(trump);
    }

    return card;
}

/**
 * @brief Try to pull some trump cards from the opponent players
 * by playing a relative low trump card
 *
 * @param player  player to choose a card to play for
 * @param list    possible cards to choose from
 *
 * @return the selected card or NULL
 */
card *truempfe_ziehen(player *player, GList *list)
{
    GList *trump = get_trump_list(list);
    GList *ptr = NULL;
    card *card = NULL;

    DPRINT(("%s: try truempfe_ziehen()\n", player->name));

    if (trump)
    {
        for (ptr = g_list_first(trump); ptr; ptr = ptr->next)
        {
            card = ptr->data;

            /* TODO: play best jack or a random one
             * play out the jacks first */
            if (card->rank == BUBE)
            {
                g_list_free(trump);
                return card;
            }
            /* play the big trumps if nothing better out there */
            else if (highest_rem_of_suit(card))
            {
                /* do not play 10 or ace if there are more jacks around */
                if ((card->rank == 10 || card->rank == ASS) &&
                        !jacks_weg(player))
                {
                    card = NULL;
                    continue;
                }

                g_list_free(trump);
                return card;
            }
        }

        g_list_free(trump);
    }

    return card;
}

/**
 * @brief Try to play a card of a short numbered non-trump suit
 *
 * @param player  player to choose a card to play for
 * @param list    possible cards to choose from
 *
 * @return the selected card or NULL
 */
card *kurz_aufspielen(player *player, GList *list)
{
    gint id = player->id;
    gint i, sel_suit = 0, min = -1, num = 0;
    GList *ptr = NULL;
    card *card = NULL;

    DPRINT(("%s: try kurz_aufspielen()\n", player->name));

    for (i=0; i<4; ++i)
    {
        if (SUITS[i] != gskat.trump)
        {
            /* skip suit if already gestochen */
            if (player->re)
            {
                if (hat_gestochen(gskat.players[(id+1)%3], SUITS[i]))
                    continue;
                if (hat_gestochen(gskat.players[(id+2)%3], SUITS[i]))
                    continue;
            }
            else
            {
                if (hat_gestochen(gskat.re, SUITS[i]))
                    continue;
            }


            ptr = get_suit_list(list, SUITS[i]);

            if (ptr)
            {
                num = g_list_length(ptr);

                if (min == -1 || num < min)
                {
                    min = num;
                    sel_suit = SUITS[i];
                }
                g_list_free(ptr);
            }
        }
    }

    if (sel_suit)
    {
        ptr = get_suit_list(list, sel_suit);

        /* TODO: implement more logic here */
        i = rand() % g_list_length(ptr);
        card = g_list_nth_data(ptr, i);

        g_list_free(ptr);
    }

    return card;
}

/**
 * @brief Try to play a card of a long-numbered suit
 *
 * @param player  player to choose a card to play for
 * @param list    possible cards to choose from
 *
 * @return the selected card or NULL
 */
card *lang_aufspielen(player *player, GList *list)
{
    gint i, sel_suit = 0, max = 0, num = 0;
    GList *ptr = NULL;
    card *card = NULL;

    DPRINT(("%s: try lang_aufspielen()\n", player->name));

    for (i=0; i<4; ++i)
    {
        if (SUITS[i] != gskat.trump)
        {
            if (!player->re)
            {
                if (hat_gestochen(gskat.re, SUITS[i]))
                    continue;
            }

            ptr = get_suit_list(list, SUITS[i]);

            if (ptr)
            {
                num = g_list_length(ptr);

                if (num > max)
                {
                    max = num;
                    sel_suit = SUITS[i];
                }
                g_list_free(ptr);
            }
        }
    }

    if (sel_suit)
    {
        ptr = get_suit_list(list, sel_suit);

        /* TODO: implement more logic here */
        i = rand() % g_list_length(ptr);
        card = g_list_nth_data(ptr, i);

        g_list_free(ptr);
    }

    return card;
}

/**
 * @brief Try to play a high points card on the Kontra team
 *
 * @param player  player to choose a card to play for
 * @param list    possible cards to choose from
 *
 * @return the selected card or NULL
 */
card *ai_kontra_schmieren(player *player, GList *list)
{
    gint i, max = 0;
    GList *ptr = NULL, *suit = NULL;
    card *card = NULL, *ret = NULL;

    DPRINT(("%s: try ai_kontra_schmieren()\n", player->name));

    card = g_list_nth_data(list, 0);

    /* player has to play trump */
    if (is_trump(card))
    {
        for (ptr = g_list_first(list); ptr; ptr = ptr->next)
        {
            card = ptr->data;

            if (card->points >= max && card->rank != BUBE)
            {
                ret = card;
                max = card->points;
            }
        }
    }
    /* select a non trump card */
    else
    {
        for (i=0; i<4; ++i)
        {
            if (SUITS[i] != gskat.trump)
            {
                if ((suit = get_suit_list(list, SUITS[i])))
                {
                    for (ptr = g_list_first(suit); ptr; ptr = ptr->next)
                    {
                        card = ptr->data;
                        if (card->rank != ASS)
                        {
                            if (card->points > max)
                            {
                                max = card->points;
                                ret = card;
                            }
                            break;
                        }
                    }
                    g_list_free(suit);
                }
            }
        }
    }

    return ret;
}

card *highest_fehl(player *player, GList *list)
{
    GList *ptr = NULL;
    card *card = NULL;

    DPRINT(("%s: try highest_fehl()\n", player->name));

    /* play only if there is no trump left */
    if (truempfe_weg(player))
    {
        for (ptr = g_list_first(list); ptr; ptr = ptr->next)
        {
            card = ptr->data;

            if (highest_rem_of_suit(card))
                return card;
        }
    }

    return NULL;
}

card *abwerfen(player *player, GList *list)
{
    gint i, min = -1, len = 0;
    gint lengths[4];
    GList *ptr = NULL;
    card *card = NULL, *ret = NULL;

    /* get lengths of card suits */
    for (i=0; i<4; ++i)
        lengths[i] = num_of_suit(list, SUITS[i]);

    DPRINT(("%s: try abwerfen()\n", player->name));

    for (ptr = g_list_last(list); ptr; ptr = ptr->prev)
    {
        card = ptr->data;

        /* skip jacks
         * TODO: not always appropriate */
        if (!gskat.null)
            if (card->rank == BUBE)
                continue;

        if (min == -1 || card->points < min ||
                (card->points == min && !highest_rem_of_suit(card) &&
                    (!is_trump(card) ||
                     lengths[(card->suit-20)/20-1] < len)))
        {
            min = card->points;
            ret = card;
            len = lengths[(card->suit-20)/20-1];
        }
    }

    return ret;
}

/* TODO: we need to consider grand and null games here */
gboolean hat_gestochen(player *player, gint suit)
{
    gint i, j;
    card **stich = NULL;

    for (i = 0; i < 10; ++i)
    {
        if ((stich = gskat.stiche[i]) == NULL)
            break;

        /* first card must not be trump or one of player's cards */
        if ((*stich)->suit != suit || (*stich)->owner == player->id
                    || is_trump(*stich))
            continue;

        for (j = 1; j < 3; ++j)
        {
            if (stich[j] && stich[j]->owner == player->id &&
                    (stich[j]->suit != suit || is_trump(stich[j])))
            {
                DPRINT(("%s hat %d gestochen\n", player->name, suit));
                return TRUE;
            }
        }
    }

    return FALSE;
}

gint num_jacks_played()
{
    gint count = 0;
    GList *ptr = NULL;

    if ((ptr = get_jack_list(gskat.played)))
    {
        count = g_list_length(ptr);
        g_list_free(ptr);
    }

    return count;
}

gboolean jacks_weg(player *player)
{
    gint count = num_jacks_played();
    GList *ptr = NULL;

    if ((ptr = get_jack_list(player->cards)))
    {
        count += g_list_length(ptr);
        g_list_free(ptr);
    }

    if (count == 4)
        return TRUE;
    return FALSE;
}

gint num_truempfe_played()
{
    gint count = 0;
    GList *ptr = NULL;
    card *card = NULL;

    /* iterate through played cards */
    for (ptr = g_list_first(gskat.played); ptr; ptr = ptr->next)
    {
        card = ptr->data;

        if (is_trump(card))
            ++count;
    }

    DPRINT(("num_truempfe_played = %d\n", count));

    return count;
}

gboolean truempfe_weg(player *player)
{
    gint count = 0;
    GList *ptr = NULL;
    card *card = NULL;

    count += num_truempfe_played();

    /* iterate through player's cards */
    for (ptr = g_list_first(player->cards); ptr; ptr = ptr->next)
    {
        card = ptr->data;

        if (is_trump(card))
            ++count;
    }

    if (count == 11 || (!gskat.trump && count == 4) || gskat.null)
        return TRUE;
    return FALSE;
}

gint len_spitzen(player *player, GList *list, gint suit)
{
    gint len = 0;
    GList *all = NULL, *ptr = NULL, *pcards = NULL;
    card *card = NULL, *cmp = NULL;

    /* get player's cards */
    if (!(pcards = get_trump_list(list)))
        return 0;

    /* get cards not played yet */
    if (suit == gskat.trump)
        all = get_trump_list(gskat.cards);
    else
        all = get_suit_list(gskat.cards, suit);

    for (ptr = g_list_first(gskat.played); ptr; ptr = ptr->next)
    {
        card = ptr->data;

        all = g_list_remove(all, card);
    }

    ptr = NULL;
    card = NULL;

    if (all)
    {
        for (ptr = g_list_first(pcards); ptr; ptr = ptr->next)
        {
            card = ptr->data;

            if ((cmp = g_list_nth_data(all, len)))
            {
                if (card == cmp)
                {
                    ++len;
                    continue;
                }
            }
            break;
        }

        g_list_free(all);
    }

    g_list_free(pcards);

    DPRINT(("%s: len_spitzen(%d) = %d\n", player->name, suit, len));

    return len;
}

gboolean muss_bedienen(player *player)
{
    card *card = NULL;

    if (gskat.table && g_list_length(gskat.table) > 0)
    {
        card = g_list_nth_data(gskat.table, 0);

        if (is_trump(card))
        {
            if (num_of_trump(player->cards) > 0)
                return TRUE;
            else
                return FALSE;
        }
        else
        {
            if (num_of_suit(player->cards, card->suit) > 0)
                return TRUE;
            else
                return FALSE;
        }
    }
    return FALSE;
}

card *highest_on_table()
{
    gint len = (gskat.table) ? g_list_length(gskat.table) : 0;

    if (gskat.table && len > 0)
    {
        if (len == 1)
            return g_list_nth_data(gskat.table, 0);
        else
        {
            if (is_greater(g_list_nth_data(gskat.table, 1),
                        g_list_nth_data(gskat.table, 0), gskat.trump, gskat.null))
                return g_list_nth_data(gskat.table, 1);
            else
                return g_list_nth_data(gskat.table, 0);
        }
    }

    return NULL;
}

gboolean kommt_drueber(player *player, GList *list)
{
    card *card = highest_on_table();

    if (is_greater(g_list_nth_data(list, 0), card, gskat.trump, gskat.null))
        return TRUE;
    return FALSE;
}

gboolean kontra_stich_sicher(player *player)
{
    card *card = NULL;

    if (g_list_length(gskat.table) == 2)
    {
        if (is_greater(g_list_nth_data(gskat.table, 1),
                    g_list_nth_data(gskat.table, 0), gskat.trump, gskat.null))
            card = g_list_nth_data(gskat.table, 1);
        else
            card = g_list_nth_data(gskat.table, 0);

        if (gskat.players[card->owner]->re)
            return FALSE;
        else
            return TRUE;
    }
    /* TODO: try to check even when there is only one card on the table */
    else
    {
        if (prob_stich_geht_durch(player) > 0.5)
            return TRUE;
    }

    return FALSE;
}

gboolean highest_rem_of_suit(card *first)
{
    gint i = 0, len;
    GList *out = cards_out();
    GList *suit = NULL;
    card *high = NULL;

    if (is_trump(first))
        suit = get_trump_list(out);
    else
        suit = get_suit_list(out, first->suit);

    g_list_free(out);

    if (suit)
    {
        len = g_list_length(suit);
        high = g_list_nth_data(suit, i);

        /* skip player's cards */
        while (i < len && high->owner == first->owner)
            high = g_list_nth_data(suit, ++i);

        g_list_free(suit);

        /* player owns all remaining cards of that suit
         * so he definitely owns the highest one as well */
        if (i == (len - 1) && high->owner == first->owner)
            return TRUE;
    }
    else
        return TRUE;

    if (high && (high == first
                || is_greater(first, high, gskat.trump, gskat.null)))
        return TRUE;
    else
        return FALSE;
}

gdouble prob_stich_geht_durch(player *player)
{
    gdouble poss = 0.25;
    gint played = 0;
    gint poss_higher = 0;

    card *first = g_list_nth_data(gskat.table, 0);

    poss_higher = num_poss_higher_cards(player, first);

    if (poss_higher == 0)
        poss = 1.0;

    else if (is_trump(first))
        played = num_of_trump(gskat.played);

    else
    {
        played = num_of_suit(gskat.played, first->suit);

        if (played == 1 && first->rank == ASS)
            poss = 0.9;

        else if (highest_rem_of_suit(first) &&
                !hat_gestochen(gskat.re, first->suit))
            poss = 0.6;

        else if (truempfe_weg(player) && hat_gestochen(gskat.re,
                    first->suit))
            poss = 0.1;

        else
            poss = 0.25;
    }

    DPRINT(("%s: prob_stich_geht_durch() == %f\n", player->name, poss));

    return poss;
}

gint num_poss_higher_cards(player *player, card *first)
{
    gint len = 0;
    GList *possible = cards_out();
    GList *ptr = NULL;
    card *new = NULL;

    if (possible)
    {
        for (ptr = g_list_first(possible); ptr; ptr = ptr->next)
        {
            new = ptr->data;

            if (is_greater(new, first, gskat.trump, gskat.null) &&
                    new->owner != player->id)
                ++len;
        }

        g_list_free(possible);
    }

    return len;
}

GList *cards_out()
{
    GList *ret = g_list_copy(gskat.cards);
    GList *ptr = NULL;
    card *card = NULL;

    for (ptr = g_list_first(gskat.played); ptr; ptr = ptr->next)
    {
        card = ptr->data;
        ret = g_list_remove(ret, card);
    }

    if (!ret || g_list_length(ret) == 0)
        return NULL;

    return ret;
}

gint punkte_auf_tisch()
{
    gint points = 0;
    GList *ptr = NULL;
    card *card = NULL;

    for (ptr = g_list_first(gskat.table); ptr; ptr = ptr->next)
    {
        card = ptr->data;

        points += card->points;
    }

    return points;
}

gint num_of_trump(GList *list)
{
    gint num = 0;
    GList *ptr = NULL;

    if ((ptr = get_trump_list(list)))
    {
        num = g_list_length(ptr);
        g_list_free(ptr);
    }
    return num;
}

gint num_of_suit(GList *list, gint suit)
{
    gint num = 0;
    GList *ptr = NULL;

    if ((ptr = get_suit_list(list, suit)))
    {
        num = g_list_length(ptr);
        g_list_free(ptr);
    }
    return num;
}

/* vim:set et sw=4 sts=4 tw=80: */
