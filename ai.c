#include <stdlib.h>
#include <gtk/gtk.h>
#include "ai.h"
#include "game.h"

static gint SUITS[] = { KREUZ, PIK, HERZ, KARO };

card *ai_select_card(app *app, player *player, GList *list)
{
    gint selection;
    gint position;
    card *card = NULL;

    /* return last card */
    if (g_list_length(list) == 1)
        return g_list_nth_data(list, 0);

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
             /* do something */
        }
        /* player is the last to play */
        else
        {
             /* do something */
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
             /* do something */
        }
        /* player is the last to play */
        else
        {
             /* do something */
        }
    }

    /* temporary fallback as long as not
     * all strategies are implemented yet */
    if (card == NULL)
    {
        selection = rand() % g_list_length(list);
        return g_list_nth_data(list, selection);
    }
    return card;
}

card *kurz_fehl_ass(app *app, GList *list)
{
    gint i, len, min = 0;
    GList *ptr = NULL;
    card *card = NULL, *ret = NULL;

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
                if (card->rank == ASS && (!min || len < min))
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

card *ai_re_kommt_raus(app *app, player *player, GList *list)
{
    card *card = NULL;

    if ((card = kurz_fehl_ass(app, list)))
        return card;

    return NULL;
}

card *ai_kontra_kommt_raus(app *app, player *player, GList *list)
{
    card *card = NULL;

    if ((card = kurz_fehl_ass(app, list)))
        return card;

    return NULL;
}

/* vim:set et sw=4 ts=4 tw=80: */
