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

void swap(gint *a, gint *b)
{
    gint tmp = *a;
    *a = *b;
    *b = tmp;
}

/* return card value depending on rank */
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

gchar *suit_name(gint id)
{
    switch (id)
    {
        case -1:
            return "Null";
        case KARO:
            return "Karo";
        case HERZ:
            return "Herz";
        case PIK:
            return "Pik";
        case KREUZ:
            return "Kreuz";
        case 0:
            return "Grand";
        default:
            return "";
    }
}

gchar *rank_name(gint id)
{
    static gchar name[4];

    switch (id)
    {
        case 1:
            return "Ass";
        case 11:
            return "Bube";
        case 12:
            return "Dame";
        case 13:
            return "König";
        default:
            g_snprintf(name, 4, "%d", id);
            return name;
    }
}

gchar *get_card_name(card *card)
{
    static gchar name[64];

    g_snprintf(name, 64, "%s %s", suit_name(card->suit), rank_name(card->rank));

    return name;
}

void print_card(card *card)
{
    g_print("%s %s", suit_name(card->suit), rank_name(card->rank));
}

void print_player_cards(player *player)
{
    GList *ptr = NULL;
    card *card = NULL;

    g_print("%s:\n", player->name);
    for (ptr = g_list_first(player->cards); ptr; ptr = ptr->next)
    {
        card = ptr->data;
        print_card(card);
        g_print(", ");
    }
    g_print("\n");
}

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

/* check if top card is higher than bottom card
 * depending on trump suit and game type */
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
