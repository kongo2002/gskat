#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gprintf.h>
#include "def.h"

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

gchar *get_card_suit(gint id, gchar *string)
{
    id -= id % 20;

    switch (id)
    {
        case 40:
            g_sprintf(string, "diamond");
            break;
        case 60:
            g_sprintf(string, "heart");
            break;
        case 80:
            g_sprintf(string, "spade");
            break;
        case 100:
            g_sprintf(string, "club");
            break;
    }

    return string;
}

gchar *get_card_rank(gint id, gchar *string)
{
    id = id % 20;

    if (id == 1)
        g_sprintf(string, "ace");
    else if (id == 11)
        g_sprintf(string, "jack");
    else if (id == 12)
        g_sprintf(string, "queen");
    else if (id == 13)
        g_sprintf(string, "king");
    else
        g_sprintf(string, "%d", id);

    return string;
}

void print_card(card *card)
{
    gchar suit[20];
    gchar rank[20];
    gint id = card->rank + card->suit;

    g_print("%s of %s", get_card_rank(id, rank),
            get_card_suit(id, suit));
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

/* vim:set et sw=4 ts=4 tw=80: */
