#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include "ai.h"
#include "game.h"
#include "utils.h"

static gint SUITS[] = { KREUZ, PIK, HERZ, KARO };

card *ai_select_card(app *app, player *player, GList *list)
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
    position = (app->table) ? g_list_length(app->table) : 0;

    /* re player */
    if (player->re)
    {
        /* player is first to play */
        if (position == 0)
        {
            card = ai_re_kommt_raus(app, player, list);
        }
        /* player is in the middle */
        else if (position == 1)
        {
            card = ai_re_mitte(app, player, list);
        }
        /* player is the last to play */
        else
        {
            card = ai_re_hinten(app, player, list);
        }
    }
    /* kontra player */
    else
    {
        /* player is first to play */
        if (position == 0)
        {
            card = ai_kontra_kommt_raus(app, player, list);
        }
        /* player is in the middle */
        else if (position == 1)
        {
            card = ai_kontra_mitte(app, player, list);
        }
        /* player is the last to play */
        else
        {
            card = ai_kontra_hinten(app, player, list);
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

card *kurz_fehl_ass(app *app, player *player, GList *list)
{
    gint i, len, min = -1;
    GList *ptr = NULL;
    card *card = NULL, *ret = NULL;

    DPRINT(("%s: try kurz_fehl_ass()\n", player->name));

    for (i=0; i<4; ++i)
    {
        if (SUITS[i] != app->trump)
        {
            if ((ptr = get_suit_list(app, list, SUITS[i])))
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

card *knapp_trumpfen(app *app, player *player, GList *list)
{
    GList *ptr = NULL;
    card *ret = NULL, *card = NULL, *high = highest_on_table(app);

    DPRINT(("%s: try knapp_trumpfen()\n", player->name));

    for (ptr = g_list_first(list); ptr; ptr = ptr->next)
    {
        card = ptr->data;

        if (is_greater(card, high, app->trump, app->null))
            ret = card;
    }
    return ret;
}

card *ai_re_kommt_raus(app *app, player *player, GList *list)
{
    card *card = NULL;

    if ((card = kurz_fehl_ass(app, player, list)))
        return card;

    if (!card)
        card = kreuz_pik_bube(app, player, list);

    return card;
}

card *ai_kontra_kommt_raus(app *app, player *player, GList *list)
{
    card *card = NULL;

    if ((card = kurz_fehl_ass(app, player, list)))
        return card;

    return NULL;
}

card *ai_re_mitte(app *app, player *player, GList *list)
{
    card *card = NULL;

    if ((card = knapp_trumpfen(app, player, list)))
        return card;
    else
        card = abwerfen(app, player, list);

    return card;
}

card *ai_kontra_mitte(app *app, player *player, GList *list)
{
    card *tmp = NULL;
    card *card = NULL;

    tmp = g_list_nth_data(app->table, 0);

    if (!app->players[tmp->owner]->re)
    {
        if (kontra_stich_sicher(app, player))
            card = ai_kontra_schmieren(app, player, list);
    }

    if (!card)
    {
        if ((card = knapp_trumpfen(app, player, list)))
            return card;
        else
            card = abwerfen(app, player, list);
    }

    return card;
}

card *ai_kontra_hinten(app *app, player *player, GList *list)
{
    card *card = NULL;

    if (kontra_stich_sicher(app, player))
        return ai_kontra_schmieren(app, player, list);
    else
        card = knapp_trumpfen(app, player, list);

    if (!card)
        card = abwerfen(app, player, list);

    return card;
}

card *ai_re_hinten(app *app, player *player, GList *list)
{
    card *card = NULL;

    if ((card = knapp_trumpfen(app, player, list)))
        return card;
    else
        card = abwerfen(app, player, list);

    return NULL;
}

card *kreuz_pik_bube(app *app, player *player, GList *list)
{
    gint prob;
    GList *ptr = get_jack_list(list);
    card *card = NULL;

    DPRINT(("%s: try kreuz_pik_bube()\n", player->name));

    if (ptr)
    {
        card = g_list_nth_data(ptr, 0);

        if (card->suit == KREUZ && g_list_length(ptr) > 1)
        {
            card = g_list_nth_data(ptr, 1);

            if (card->suit == PIK)
            {
                prob = rand() % 2;
                card = g_list_nth_data(ptr, prob);

                g_list_free(ptr);

                return card;
            }
        }

        g_list_free(ptr);
    }

    return NULL;
}

/* TODO: should also work for trump */
card *ai_kontra_schmieren(app *app, player *player, GList *list)
{
    gint i, max = 0;
    GList *ptr = NULL, *suit = NULL;
    card *card = NULL, *ret = NULL;

    DPRINT(("%s: try ai_kontra_schmieren()\n", player->name));

    for (i=0; i<4; ++i)
    {
        if (SUITS[i] != app->trump)
        {
            if ((suit = get_suit_list(app, list, SUITS[i])))
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

    return ret;
}

card *abwerfen(app *app, player *player, GList *list)
{
    gint min = -1;
    GList *ptr = NULL;
    card *card = NULL, *ret = NULL;

    DPRINT(("%s: try abwerfen()\n", player->name));

    for (ptr = g_list_last(list); ptr; ptr = ptr->prev)
    {
        card = ptr->data;

        if (min == -1 || card->points < min ||
                (card->points == min && card->rank < ret->rank))
        {
            min = card->points;
            ret = card;
        }
    }

    return ret;
}

/* TODO: we need to consider grand and null games here */
gboolean hat_gestochen(app *app, player *player, gint suit)
{
    gint i = 0;
    gboolean found = FALSE;
    GList *ptr = NULL;
    card *card = NULL;

    for (ptr = g_list_first(app->played); ptr; ptr = ptr->next)
    {
        card = ptr->data;

        if (++i > 3)
        {
            found = FALSE;
            i = 1;
        }

        if (i == 1 && card->suit == suit && card->rank != BUBE &&
                card->owner != player->id)
        {
            found = TRUE;
            continue;
        }

        if (found == TRUE && card->owner == player->id &&
                (card->suit != suit || card->rank == BUBE))
            return TRUE;
    }

    return FALSE;
}

gint num_truempfe_draussen(app *app)
{
    gint count = 0;
    GList *ptr = NULL;
    card *card = NULL;

    /* iterate through played cards */
    for (ptr = g_list_first(app->played); ptr; ptr = ptr->next)
    {
        card = ptr->data;

        if (is_trump(app, card))
            ++count;
    }

    return count;
}

gboolean truempfe_draussen(app *app, player *player)
{
    gint count = 0;
    GList *ptr = NULL;
    card *card = NULL;

    count += num_truempfe_draussen(app);

    /* iterate through player's cards */
    for (ptr = g_list_first(player->cards); ptr; ptr = ptr->next)
    {
        card = ptr->data;

        if (is_trump(app, card))
            ++count;
    }

    if (count == 11)
        return TRUE;
    return FALSE;
}

gboolean muss_bedienen(app *app, player *player)
{
    card *card = NULL;

    if (app->table && g_list_length(app->table) > 0)
    {
        card = g_list_nth_data(app->table, 0);

        if (is_trump(app, card))
        {
            if (num_of_trump(app, player->cards) > 0)
                return TRUE;
            else
                return FALSE;
        }
        else
        {
            if (num_of_suit(app, player->cards, card->suit) > 0)
                return TRUE;
            else
                return FALSE;
        }
    }
    return FALSE;
}

card *highest_on_table(app *app)
{
    gint len = 0;

    if (app->table && (len = g_list_length(app->table) > 0))
    {
        if (len == 1)
            return g_list_nth_data(app->table, 0);
        else if (len == 2)
        {
            if (is_greater(g_list_nth_data(app->table, 1),
                        g_list_nth_data(app->table, 0), app->trump, app->null))
                return g_list_nth_data(app->table, 1);
            else
                return g_list_nth_data(app->table, 0);
        }
    }
    return NULL;
}

gboolean kommt_drueber(app *app, player *player, GList *list)
{
    card *card = highest_on_table(app);

    if (is_greater(g_list_nth_data(list, 0), card, app->trump, app->null))
        return TRUE;
    return FALSE;
}

gboolean kontra_stich_sicher(app *app, player *player)
{
    card *card = NULL;

    if (g_list_length(app->table) == 2)
    {
        if (is_greater(g_list_nth_data(app->table, 1),
                    g_list_nth_data(app->table, 0), app->trump, app->null))
            card = g_list_nth_data(app->table, 1);
        else
            card = g_list_nth_data(app->table, 0);

        if (app->players[card->owner]->re)
            return FALSE;
        else
            return TRUE;
    }
    /* TODO: try to check even when there is only one card on the table */
    else
    {
        if (prob_stich_geht_durch(app, player) > 0.5)
            return TRUE;
    }

    return FALSE;
}

gboolean highest_rem_of_suit(app *app, card *first)
{
    GList *out = cards_out(app);
    GList *suit = NULL;
    card *high = NULL;

    if (is_trump(app, first))
        suit = get_trump_list(app, out);
    else
        suit = get_suit_list(app, out, first->suit);

    if (suit)
    {
        high = g_list_nth_data(suit, 0);
        g_list_free(suit);
    }

    g_list_free(out);

    if (high && high == first)
        return TRUE;
    else
        return FALSE;
}

gdouble prob_stich_geht_durch(app *app, player *player)
{
    gdouble poss;
    gint played = 0;
    gint poss_higher = 0;

    card *first = g_list_nth_data(app->table, 0);

    poss_higher = num_poss_higher_cards(app, player, first);

    if (poss_higher == 0)
        poss = 1.0;

    else if (is_trump(app, first))
        played = num_of_trump(app, app->played);

    else
    {
        played = num_of_suit(app, app->played, first->suit);

        if (played == 1 && first->rank == ASS)
            poss = 0.9;

        else if (highest_rem_of_suit(app, first) &&
                !hat_gestochen(app, app->re, first->suit))
            poss = 0.6;

        else if (truempfe_draussen(app, player) && hat_gestochen(app, app->re,
                    first->suit))
            poss = 0.1;

        else
            poss = 0.25;
    }

    DPRINT(("%s: prob_stich_geht_durch() == %f\n", player->name, poss));

    return poss;
}

gint num_poss_higher_cards(app *app, player *player, card *first)
{
    gint len = 0;
    GList *possible = cards_out(app);
    GList *ptr = NULL;
    card *new = NULL;

    if (possible)
    {
        for (ptr = g_list_first(possible); ptr; ptr = ptr->next)
        {
            new = ptr->data;

            if (is_greater(new, first, app->trump, app->null) &&
                    new->owner != player->id)
                ++len;
        }

        g_list_free(possible);
    }

    return len;
}

GList *cards_out(app *app)
{
    GList *ret = g_list_copy(app->cards);
    GList *ptr = NULL;
    card *card = NULL;

    for (ptr = g_list_first(app->played); ptr; ptr = ptr->next)
    {
        card = ptr->data;
        ret = g_list_remove(ret, card);
    }

    if (!ret || g_list_length(ret) == 0)
        return NULL;

    return ret;
}

gint num_of_trump(app *app, GList *list)
{
    int num = 0;
    GList *ptr = NULL;

    if ((ptr = get_trump_list(app, list)))
    {
        num = g_list_length(ptr);
        g_list_free(ptr);
    }
    return num;
}

gint num_of_suit(app *app, GList *list, gint suit)
{
    int num = 0;
    GList *ptr = NULL;

    if ((ptr = get_suit_list(app, list, suit)))
    {
        num = g_list_length(ptr);
        g_list_free(ptr);
    }
    return num;
}

/* vim:set et sw=4 ts=4 tw=80: */
