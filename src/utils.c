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

#define GSKAT_CONTEXTID 42

/**
 * @brief Helper function to swap two integers
 */
void swap(gint *a, gint *b)
{
    gint tmp = *a;
    *a = *b;
    *b = tmp;
}

/**
 * @brief Return maximum string length
 */
gint max_str_len(const gchar *first, ...)
{
    gint max = strlen(first);
    gchar *str = NULL;
    va_list args;

    va_start(args, first);

    while ((str = va_arg(args, gchar *)))
    {
        if (strlen(str) > max)
            max = strlen(str);
    }

    va_end(args);

    return max;
}

/**
 * @brief Get card value depending on its rank
 */
gint get_card_points(gint rank)
{
    switch (rank)
    {
        case ASS:
            return 11;
        case 7:
        case 8:
        case 9:
            return 0;
        case 10:
            return 10;
        case BUBE:
            return 2;
        case DAME:
            return 3;
        case KOENIG:
            return 4;
    }
    return 0;
}

/**
 * @brief Return the card's suit name
 */
gchar *suit_name(gint id)
{
    switch (id)
    {
        case -1:
            return _("Null");
        case KARO:
            return _("Diamonds");
        case HERZ:
            return _("Hearts");
        case PIK:
            return _("Spades");
        case KREUZ:
            return _("Clubs");
        case 0:
            return _("Grand");
        default:
            return "";
    }
}

/**
 * @brief Return the card's rank name
 */
gchar *rank_name(gint id)
{
    static gchar name[4];

    switch (id)
    {
        case 1:
            return _("Ace");
        case 11:
            return _("Jack");
        case 12:
            return _("Queen");
        case 13:
            return _("King");
        default:
            g_snprintf(name, 4, "%d", id);
            return name;
    }
}

/**
 * @brief Convenience wrapper for the get_card_ptr function
 *
 * @param id  id of card to fetch
 *
 * @return card* of found card or NULL if no matching card was found
 */
card *get_card_by_id(gint id)
{
    gint rank = id % 20;
    gint suit = id - rank;

    return get_card_ptr(suit, rank);
}

/**
 * @brief Get pointer to the card of given suit and rank
 *
 * @param suit  suit of card to fetch
 * @param rank  rank of card to fetch
 *
 * @return card* of found card or NULL if no matching card was found
 */
card *get_card_ptr(gint suit, gint rank)
{
    GList *ptr = NULL;
    card *tmp;

    if (!gskat.cards)
        return NULL;

    for (ptr = g_list_first(gskat.cards); ptr; ptr = ptr->next)
    {
        tmp = (card *) ptr->data;

        if (tmp && tmp->suit == suit && tmp->rank == rank)
            return tmp;
    }

    return NULL;
}

/**
 * @brief Return the card's name
 *
 * @param card  pointer to card to get the name of
 *
 * @return string of card name (static gchar*)
 */
gchar *get_card_name(card *card)
{
    static gchar name[64];

    g_snprintf(name, 64, "%s %s", suit_name(card->suit), rank_name(card->rank));

    return name;
}

/**
 * @brief Print the card's name to stdout
 *
 * @param card  pointer to card to print
 */
void print_card(card *card)
{
    g_print("%s %s", suit_name(card->suit), rank_name(card->rank));
}

/**
 * @brief Print player's cards to stdout
 *
 * @param player pointer to player to print
 */
void print_player_cards(player *player)
{
    GList *ptr = NULL;
    card *card = NULL;

    g_print("%s:\n", player->name);
    for (ptr = g_list_first(player->cards); ptr; ptr = ptr->next)
    {
        if (card)
            g_print(", ");
        card = ptr->data;
        print_card(card);
    }
    g_print("\n");
}

/**
 * @brief Remove the current message from the statusbar stack and
 * push a new message onto the message stack
 *
 * @param fmt  printf-like format-string
 * @param ...  additional arguments
 */
void update_sb(const gchar *fmt, ...)
{
    gchar *msg = NULL;
    GtkStatusbar *sb = (GtkStatusbar *) gskat.widgets[14];
    va_list args;

    if (sb)
    {
        va_start(args, fmt);
        msg = g_strdup_vprintf(fmt, args);

        if (msg)
        {
            gtk_statusbar_pop(sb, GSKAT_CONTEXTID);
            gtk_statusbar_push(sb, GSKAT_CONTEXTID, msg);

            g_free(msg);
        }

        va_end(args);
    }
}

/**
 * @brief Compare two cards' ranks
 *
 * @param top     card lying on the top
 * @param bottom  card lying on the bottom
 *
 * @return TRUE if top is higher that bottom, otherwise FALSE
 */
gboolean higher_rank(card *top, card *bottom)
{
    /* top = jack */
    if (top->rank == BUBE)
    {
        if (bottom->rank != BUBE)
            return TRUE;
        else if (top->suit > bottom->suit)
            return TRUE;
        else
            return FALSE;
    }
    /* no jack */
    else if (bottom->rank != BUBE)
    {
        /* ace */
        if (top->rank == ASS && bottom->rank != ASS)
            return TRUE;
        else if (bottom->rank == ASS)
            return FALSE;
        /* ten */
        else if (top->rank == 10 && bottom->rank != 10)
            return TRUE;
        else if (bottom->rank == 10)
            return FALSE;
        else if (top->rank > bottom->rank)
            return TRUE;
        else
            return FALSE;
    }
    /* bottom = jack */
    else
        return FALSE;
}

/**
 * @brief Check if the top card is higher than the bottom card
 * depending on trump and game type
 *
 * @param top card lying on the top
 * @param bottom card lying on the bottom
 * @param trump trump suit index
 * @param null TRUE if it's a null game, otherwise FALSE
 *
 * @return TRUE if top is greater than bottom, otherwise FALSE
 */
gboolean is_greater(card *top, card *bottom, gint trump, gboolean null)
{
    /* null (ramsch) */
    if (null)
    {
        if (top->suit != bottom->suit)
            return FALSE;
        else if (top->rank == ASS)
            return TRUE;
        else if (bottom->rank == ASS)
            return FALSE;
        else if (top->rank > bottom->rank)
            return TRUE;
        else
            return FALSE;
    }
    /* grand */
    else if (!trump)
    {
        if (top->rank == BUBE)
        {
            if (bottom->rank != BUBE)
                return TRUE;
            else if (higher_rank(top, bottom))
                return TRUE;
            else
                return FALSE;
        }
        else
        {
            if (bottom->rank == BUBE)
                return FALSE;
            else if (top->suit != bottom->suit)
                return FALSE;
            else if (higher_rank(top, bottom))
                return TRUE;
            else
                return FALSE;
        }
    }
    /* default game */
    else
    {
        /* top = trump */
        if ((top->suit == trump || top->rank == BUBE) &&
                (bottom->suit != trump && bottom->rank != BUBE))
            return TRUE;
        /* bottom = trump */
        else if ((top->suit != trump && top->rank != BUBE) &&
                (bottom->suit == trump || bottom->rank == BUBE))
            return FALSE;
        /* both trump */
        else if ((top->suit == trump || top->rank == BUBE) &&
                (bottom->suit == trump || bottom->rank == BUBE))
        {
            if (top->rank == BUBE)
            {
                if (bottom->rank != BUBE)
                    return TRUE;
                else if (top->suit > bottom->suit)
                    return TRUE;
                else
                    return FALSE;
            }
            else
            {
                if (bottom->rank == BUBE)
                    return FALSE;
                else if (higher_rank(top, bottom))
                    return TRUE;
                else
                    return FALSE;
            }
        }
        /* no trump */
        else
        {
            if (top->suit != bottom->suit)
                return FALSE;
            else if (higher_rank(top, bottom))
                return TRUE;
            else
                return FALSE;
        }
    }
}

/* vim:set et sw=4 sts=4 tw=80: */
