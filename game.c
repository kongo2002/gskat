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

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include "def.h"
#include "ai.h"
#include "interface.h"
#include "game.h"
#include "utils.h"

void card_to_player(player *player, card *card)
{
    if (player->human)
        card->draw_face = TRUE;
    else
        card->draw_face = FALSE;

    card->draw = TRUE;
    card->status = CS_DISTRIBUTED;
    card->owner = player->id;

    player->cards = g_list_prepend(player->cards, (gpointer) card);
}

void give_to_skat(app *app, card *card)
{
    card->draw = TRUE;
    card->draw_face = FALSE;
    card->status = CS_DISTRIBUTED;
    card->owner = -1;

    app->skat = g_list_prepend(app->skat, (gpointer) card);
}

gboolean play_card(app *app, GdkEventButton *event)
{
    gint num_cards = (app->table) ? g_list_length(app->table) : 0;
    GList *ptr = NULL;
    card *card = NULL;
    player *player = app->players[0];

    /* check if it's the player's turn */
    if (((app->player + num_cards) % 3) == 0)
    {
        /* get card that has been clicked on */
        if ((card = click_card(app, event, player->cards)))
        {
            /* check if card is possible */
            ptr = get_possible_cards(app, player->cards);
            if (g_list_index(ptr, card) != -1)
            {
                throw_card(app, card);
                play_stich(app);
                return TRUE;
            }
            else
                DPRINT(("Card is not possible.\n"));
        }
    }

    return FALSE;
}

gboolean click_skat(app *app, GdkEventButton *event)
{
    player *player = app->players[0];
    card *card = NULL;

    if ((card = click_card(app, event, player->cards)))
    {
        /* swap clicked card with last card in skat */
        app->skat = g_list_prepend(app->skat, (gpointer) card);
        player->cards = g_list_remove(player->cards, (gconstpointer) card);

        card = g_list_nth_data(app->skat, 2);
        player->cards = g_list_prepend(player->cards, (gpointer) card);
        app->skat = g_list_remove(app->skat, (gconstpointer) card);

        player->cards = g_list_sort_with_data(player->cards,
                compare_cards, app);

        /* redraw screen */
        calc_card_positions(app);
        draw_area(app);

        return TRUE;
    }

    return FALSE;
}

card *click_card(app *app, GdkEventButton *event, GList *list)
{
    GList *ptr = NULL;
    card *card = NULL;

    for (ptr = g_list_last(list); ptr; ptr = ptr->prev)
    {
        card = ptr->data;

        if ((gint) event->x >= card->dim.x
                && (gint) event->y >= card->dim.y
                && (gint) event->x < card->dim.x + card->dim.w
                && (gint) event->y < card->dim.y + card->dim.h)
        {
            print_card(card);
            return card;
        }
    }
    return NULL;
}

void give_cards(app *app)
{
    gint order[32];
    gint i, j, k;
    player *player;

    /* init order list */
    for (i=0; i<32; ++i)
        order[i] = i;

    /* generate random card order */
    srand((unsigned) time(NULL));
    for (i=0; i<100; ++i)
    {
        j = rand()%32;
        do
            k = rand()%32;
        while (k == j);
        swap(&order[j], &order[k]);
    }

    /* give cards to players */
    for (i=0; i<3; ++i)
    {
        player = app->players[i];
        for (j=0; j<10; ++j)
        {
            card_to_player(player,
                    g_list_nth_data(app->cards, order[j+(i*10)]));
        }
        player->cards = g_list_sort_with_data(player->cards,
                compare_cards, app);
    }

    /* two cards for skat */
    give_to_skat(app, g_list_nth_data(app->cards, order[30]));
    give_to_skat(app, g_list_nth_data(app->cards, order[31]));
}

gint get_provoke_response(app *app, gint value, gchar *msg, gboolean hoeren)
{
    gint result;
    gchar caption[4];
    g_sprintf(caption, "%d", value);

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Reizen",
            GTK_WINDOW(app->allwidgets[0]),
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            (hoeren) ? "Ja" : caption, value,
            (hoeren) ? "Nein" : "Passen", 0,
            NULL);

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
            gtk_label_new(msg), FALSE, TRUE, 2);
    gtk_widget_show_all(dialog);

    result = gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);
    return result;
}

gint next_reizwert(gint value)
{
    gint i;
    gint reizen[] = { 18,20,22,23,24,27,30,33,35,36,40,44,45,46,
                      48,50,54,55,59,60,63,66,70,72,77,80,81,84,
                      88,90,96,99,100,108,110,117,120,121,126,130,
                      132,135,140,143,144,150,153,154,156,160,162,
                      165,168,170,176,180,187,192,198,204,216,240,264};

    for (i=0; i<62; ++i)
        if (value == reizen[i])
            return reizen[i+1];
    return 0;
}

gint compare_cards(gconstpointer a, gconstpointer b, gpointer data)
{
    card *card_a = (card *) a;
    card *card_b = (card *) b;
    struct _app *app = (struct _app *) data;

    if (!app->null)
    {
        if (card_a->rank == BUBE && card_b->rank == BUBE)
        {
            if (card_a->suit > card_b->suit)
                return -1;
            else
                return 1;
        }
        else if (card_a->rank == BUBE)
            return -1;
        else if (card_b->rank == BUBE)
            return 1;
    }

    /* change default order when trump given */
    if (app->trump > 0)
    {
        if (card_a->suit == app->trump && card_b->suit != app->trump)
            return -1;
        else if (card_a->suit != app->trump && card_b->suit == app->trump)
            return 1;
    }

    if (card_a->suit > card_b->suit)
        return -1;
    else if (card_a->suit == card_b->suit)
    {
        if (card_a->rank == ASS)
            return -1;
        else if (card_b->rank == ASS)
            return 1;
        else if (card_a->rank == 10)
            return -1;
        else if (card_b->rank == 10)
            return 1;
        else if (card_a->rank > card_b->rank)
            return -1;
        else
            return 1;
    }
    else
        return 1;

    return 0;
}

gint compare_jacks(gconstpointer a, gconstpointer b)
{
    card *card_a = (card *) a;
    card *card_b = (card *) b;

    if (card_a->suit > card_b->suit)
        return -1;
    else if (card_a->suit < card_b->suit)
        return 1;
    else
        return 0;
}

gint compare_family(gconstpointer a, gconstpointer b)
{
    card *card_a = (card *) a;
    card *card_b = (card *) b;

    if (card_a->rank == ASS)
        return -1;
    else if (card_b->rank == ASS)
        return 1;
    else if (card_a->rank == 10)
        return -1;
    else if (card_b->rank == 10)
        return 1;
    else if (card_a->rank > card_b->rank)
        return -1;
    else if (card_a->rank < card_b->rank)
        return 1;
    else
        return 0;
}

/* return a list of jacks of a given list of cards */
GList *get_jack_list(GList *list)
{
    GList *ptr = NULL, *jacks = NULL;
    card *card = NULL;

    for (ptr = g_list_first(list); ptr; ptr = ptr->next)
    {
        card = ptr->data;
        if (card->rank == BUBE)
            jacks = g_list_prepend(jacks, (gpointer) card);
    }

    if (jacks)
        jacks = g_list_sort(jacks, compare_jacks);
    return jacks;
}

GList *get_suit_list(app *app, GList *list, gint suit)
{
    GList *ptr = NULL, *family = NULL;
    card *card = NULL;

    for (ptr = g_list_first(list); ptr; ptr = ptr->next)
    {
        card = ptr->data;

        if (!app->null)
        {
            if (card->suit == suit && card->rank != BUBE)
                family = g_list_prepend(family, (gpointer) card);
        }
        else
        {
            if (card->suit == suit)
                family = g_list_prepend(family, (gpointer) card);
        }
    }

    if (family)
        family = g_list_sort(family, compare_family);
    return family;
}

gboolean is_trump(app *app, card *card)
{
    if (app->null)
        return FALSE;
    if (!app->trump)
    {
        if (card->rank == BUBE)
            return TRUE;
    }
    else
    {
        if (card->suit == app->trump || card->rank == BUBE)
            return TRUE;
    }
    return FALSE;
}

GList *get_trump_list(app *app, GList *list)
{
    GList *ptr = NULL, *family = NULL, *jacks = NULL;
    card *card = NULL;

    if (app->null)
        return NULL;

    jacks = get_jack_list(list);

    if (!app->trump)
        return jacks;

    family = get_suit_list(app, list, app->trump);

    if (jacks)
    {
        for (ptr = g_list_last(jacks); ptr; ptr = ptr->prev)
        {
            card = ptr->data;
            family = g_list_prepend(family, card);
        }

        g_list_free(jacks);
    }

    return family;
}

GList *get_possible_cards(app *app, GList *list)
{
    GList *ptr = NULL;
    card *card = NULL;

    /* all cards are possible if there is no card on the table
     * copy the list because the returned list will be freed after usage */
    if (app->table == NULL || g_list_length(app->table) == 0)
        return g_list_copy(list);
    else
    {
        card = g_list_nth_data(app->table, 0);

        if (is_trump(app, card))
            ptr = get_trump_list(app, list);
        else
            ptr = get_suit_list(app, list, card->suit);

        if (ptr)
            return ptr;
        else
            return g_list_copy(list);
    }
    return NULL;
}

gint rate_cards(app *app, player *player, GList *list)
{
    gint i, jacks = 0, rate = 0, best = 0;
    GList *ptr = NULL;
    card *card = NULL;

    best = get_best_suit(list);

    /* +1 if forehand */
    if (app->forehand == player->id)
        rate++;

    /* no or one card of one suit? */
    for (i=0; i<4; ++i)
    {
        ptr = get_suit_list(app, list, SUITS[i]);

        if (!ptr)
            rate++;
        else
        {
            if (g_list_length(ptr) == 1)
                rate++;
            g_list_free(ptr);
        }
    }

    for (ptr = g_list_first(list); ptr; ptr = ptr->next)
    {
        card = ptr->data;
        if (card->rank == BUBE)
            jacks++;
        else if (card->suit == best || card->rank == ASS)
            rate++;
    }

    return (rate + jacks*1.5);
}

/* find the best suit in a list of cards
 * TODO: select best suit if several suits
 * of the same count */
gint get_best_suit(GList *list)
{
    gint i, max, ret, rank_count[4];
    GList *ptr = NULL;
    card *card = NULL;

    for (i=0; i<4; ++i)
        rank_count[i] = 0;

    for (ptr = g_list_first(list); ptr; ptr = ptr->next)
    {
        card = ptr->data;
        if (card->rank != BUBE)
            rank_count[(card->suit-20)/20-1] += 1;
    }

    max = 0;
    for (i=0; i<4; ++i)
    {
        if (rank_count[i] >= max)
        {
            max = rank_count[i];
            ret = i;
        }
    }
    return ((ret+1)*20+20);
}

gint get_spitzen(app *app, GList *list, gint suit)
{
    gint i = 0, max = 0, back = 0;
    GList *ptr = NULL, *cards = NULL, *pcards = NULL;
    gboolean mit = FALSE;
    card *card = NULL, *cmp = NULL;

    /* backup trump */
    back = app->trump;
    app->trump = suit;

    cards = get_trump_list(app, app->cards);
    pcards = get_trump_list(app, list);

    for (ptr = g_list_first(cards); ptr; ptr = ptr->next)
    {
        cmp = ptr->data;

        if ((card = g_list_nth_data(pcards, i)))
        {
            if (card == cmp)
            {
                if (max == 0)
                    mit = TRUE;
                else if (mit == FALSE)
                    break;

                max++;
                i++;
                continue;
            }
            else if (mit == FALSE)
            {
                max++;
                continue;
            }
        }
        break;
    }

    g_list_free(cards);

    if (pcards)
        g_list_free(pcards);

    /* restore trump */
    app->trump = back;

    return (max + 1);
}

gint get_max_reizwert(app *app, GList *list)
{
    gint suit = 0;
    gint max = 0;

    suit = get_best_suit(list);

    max = get_spitzen(app, list, suit);

    switch (suit)
    {
        case KARO:
            return max * 9;
        case HERZ:
            return max * 10;
        case PIK:
            return max * 11;
        case KREUZ:
            return max * 12;
    }
    return 0;
}

gint do_hoeren(app *app, player *player, gint value, gint sager)
{
    gint max = 0;
    gint response = 0;
    gchar *msg;

    if (player->human)
    {
        msg = (gchar *) g_malloc(sizeof(gchar) *
                (20+strlen(app->player_names[sager])));
        g_sprintf(msg, "%s sagt %d:", app->player_names[sager], value);

        response = get_provoke_response(app, value, msg, TRUE);

        g_free(msg);
    }
    else
    {
        max = get_max_reizwert(app, player->cards);

        if (rate_cards(app, player, player->cards) >= 7 && value <= max)
        {
            player->gereizt = value;
            return value;
        }
        else
            return 0;
    }

    if (response)
        player->gereizt = value;
    return response;
}

gint do_sagen(app *app, player *player, gint hoerer, gint value)
{
    gint response = 0;
    gint gereizt = 0;
    gint id = 0;
    gint max = 0;
    gchar *msg;

    DPRINT(("Sager: %s; Hoerer: %s\n", player->name,
                app->player_names[hoerer]));

    /* pass immediately? */
    if (value != player->gereizt)
    {
        if (player->human)
        {
            msg = (gchar *) g_malloc(sizeof(gchar) *
                    (20+strlen(app->player_names[hoerer])));
            g_sprintf(msg, "Hoerer: %s. Sagen?", app->player_names[hoerer]);

            gereizt = get_provoke_response(app, value, msg, FALSE);

            g_free(msg);
        }
        else
        {
            max = get_max_reizwert(app, player->cards);

            if (rate_cards(app, player, player->cards) >= 7 && value <= max)
                gereizt = value;
            else
                gereizt = 0;
        }
    }
    else
        gereizt = player->gereizt;

    /* hoeren */
    if (gereizt)
    {
        DPRINT(("%s sagt %d\n", player->name, value));
        player->gereizt = value;

        response = do_hoeren(app, app->players[hoerer], value, player->id);
        DPRINT(("%s sagt %s\n", app->player_names[hoerer],
                (response) ? "JA" : "NEIN"));

        if (response)
        {
            id = do_sagen(app, player, hoerer, next_reizwert(value));
        }
        else
            return player->id;
    }

    if (player->gereizt == app->players[hoerer]->gereizt)
        return hoerer;
    return id;
}

void start_provoke(app *app)
{
    gchar msg[4];
    gint hoerer = app->forehand;
    gint sager = (hoerer + 1) % 3;
    gint i = 18;

    DPRINT(("Start of provoking\n"));

    /* disable button */
    gtk_widget_set_sensitive(app->allwidgets[1], FALSE);

    /* reset all player values */
    for (i=0; i<3; ++i)
    {
        app->players[i]->gereizt = 0;

        DPRINT(("MaxReizwert of %s: %d\n", app->player_names[i],
                    get_max_reizwert(app, app->players[i]->cards)));
        DPRINT(("CardRating of %s: %d\n", app->player_names[i],
                    rate_cards(app, app->players[i], app->players[i]->cards)));
    }

    /* sagen */
    sager = do_sagen(app, app->players[sager], hoerer, 18);
    DPRINT(("%s won 1. reizen with %d\n", app->player_names[sager],
            app->players[sager]->gereizt));

    sager = do_sagen(app, app->players[sager], (hoerer+2) % 3,
            (app->players[sager]->gereizt) ? app->players[sager]->gereizt : 18);

    /* first two players have passed */
    if (app->players[sager]->gereizt == 0)
        do_hoeren(app, app->players[sager], 18, sager);

    /* TODO: Ramsch */
    if (app->players[sager]->gereizt)
    {
        DPRINT(("%s won 2. reizen with %d\n", app->player_names[sager],
                    app->players[sager]->gereizt));

        app->re = app->players[sager];
        app->re->re = TRUE;

        /* update interface */
        gtk_widget_set_sensitive(app->allwidgets[1], TRUE);
        gtk_button_set_label(GTK_BUTTON(app->allwidgets[1]), "Spiel ansagen");
        gtk_label_set_text(GTK_LABEL(app->allwidgets[3]), app->re->name);

        g_sprintf(msg, "%d", app->re->gereizt);
        gtk_label_set_text(GTK_LABEL(app->allwidgets[5]), msg);

        take_skat(app);
    }
    else
    {
        DPRINT(("All players have passed -> Ramsch.\n"));

        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(app->allwidgets[0]),
                GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_INFO,
                GTK_BUTTONS_CLOSE,
                "Alle Spieler haben gepasst.\nNeue Runde");

        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        reset_game(app);
        game_start(app);
    }
}

/* TODO: select cards to be put in skat
 * combine with spiel_ansagen() */
void druecke_skat(app *app)
{
    gint suits[] = { 40, 60, 80, 100 };
    gint count = 0;
    gint i, best;
    GList *ptr = NULL;
    card *card = NULL;
    player *player = app->re;

    /* add skat to player's cards */
    player->cards = g_list_prepend(player->cards,
            g_list_nth_data(app->skat, 0));
    player->cards = g_list_prepend(player->cards,
            g_list_nth_data(app->skat, 1));

    /* empty skat */
    g_list_free(app->skat);
    app->skat = NULL;

    best = get_best_suit(player->cards);

    /* search for a suit with only 1 card left
     * when this card is no ASS -> take it */
    for (i=0; i<4; ++i)
    {
        if (suits[i] != best && count < 2)
        {
            ptr = get_suit_list(app, player->cards, suits[i]);

            if (g_list_length(ptr) == 1)
            {
                card = g_list_nth_data(ptr, 0);
                if (card->rank != ASS)
                {
                    app->skat = g_list_prepend(app->skat, card);
                    player->cards = g_list_remove(player->cards, card);
                    ++count;
                }
            }

            g_list_free(ptr);
        }
    }

    /* search for a suit with only 2 cards left
     * if none of them is an ace -> take 'em */
    if (count == 0)
    {
        for (i=0; i<4; ++i)
        {
            if (suits[i] != best)
            {
                ptr = get_suit_list(app, player->cards, suits[i]);
    
                if (g_list_length(ptr) == 2)
                {
                    card = g_list_nth_data(ptr, 0);
                    if (card->rank == ASS)
                        continue;
    
                    app->skat = g_list_prepend(app->skat, card);
                    player->cards = g_list_remove(player->cards, card);
    
                    card = g_list_nth_data(ptr, 1);
                    app->skat = g_list_prepend(app->skat, card);
                    player->cards = g_list_remove(player->cards, card);
    
                    count = 2;
                    break;
                }

                g_list_free(ptr);
            }
        }
    }

    /* select remaining amount of cards to be added to skat */
    while (count < 2)
    {
        gint len = 0;
        gint min = 10;
        GList *minimum = NULL;

        for (i=0; i<4; ++i)
        {
            if (suits[i] != best)
            {
                ptr = get_suit_list(app, player->cards, suits[i]);

                len = g_list_length(ptr);
                if (len < min && len > 0)
                {
                    if (len == 1)
                    {
                        card = g_list_nth_data(ptr, 0);
                        if (card->rank == ASS)
                        {
                            g_list_free(ptr);
                            continue;
                        }
                    }

                    if (minimum)
                        g_list_free(minimum);

                    min = len;
                    minimum = ptr;
                }
                else if (ptr)
                    g_list_free(ptr);
            }
        }

        ptr = g_list_last(minimum);
        card = ptr->data;

        app->skat = g_list_prepend(app->skat, card);
        player->cards = g_list_remove(player->cards, card);
        ++count;

        if (minimum)
            g_list_free(minimum);
    }

#ifdef DEBUG
    print_card(g_list_nth_data(app->skat, 0)); g_print("\n");
    print_card(g_list_nth_data(app->skat, 1)); g_print("\n");
#endif
}

void take_skat(app *app)
{
    gint result;
    GList *ptr = NULL;
    card *card = NULL;

    if (app->re->human)
    {
        do
        {
            GtkWidget *dialog = gtk_dialog_new_with_buttons("Hand spielen?",
                    GTK_WINDOW(app->allwidgets[0]),
                    GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
                    "Ja", 1,
                    "Nein", 0,
                    NULL);

            gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
                    gtk_label_new("Hand spielen?"), FALSE, TRUE, 2);
            gtk_widget_show_all(dialog);

            result = gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }
        while (result == GTK_RESPONSE_DELETE_EVENT);

        if (result == 1)
            app->hand = TRUE;
    }

    app->state = TAKESKAT;

    for (ptr = g_list_first(app->skat); ptr; ptr = ptr->next)
    {
        /* update card owner */
        card = ptr->data;
        card->owner = app->re->id;

        /* show cards in skat */
        if (app->re->human && !app->hand)
        {
            card->draw = TRUE;
            card->draw_face = TRUE;
        }
    }

    if (!app->re->human)
    {
        druecke_skat(app);
        spiel_ansagen(app);

        app->state = PLAYING;
    }

    /* update screen */
    draw_area(app);
}

void spiel_ansagen(app *app)
{
    gint result = 0;
    gchar gamename[10];
    GList *list = NULL;
    card *card = NULL;

    DPRINT(("Spiel ansagen.\n"));

    /* select game to play */
    if (app->re->human)
    {
        do
        {
            GtkWidget *dialog = gtk_dialog_new_with_buttons("Spiel ansagen",
                    GTK_WINDOW(app->allwidgets[0]),
                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                    "Kreuz", KREUZ,
                    "Pik", PIK,
                    "Herz", HERZ,
                    "Karo", KARO,
                    "Grand", 200,
                    "Null", 0,
                    NULL);

            result = gtk_dialog_run(GTK_DIALOG(dialog));

            gtk_widget_destroy(dialog);
        }
        while (result == GTK_RESPONSE_DELETE_EVENT);

        /* set game/trump */
        if (result == 200)
            app->trump = 0;
        else if (result == 0)
        {
            app->trump = -1;
            app->null = TRUE;
        }
        else
            app->trump = result;
    }
    else
    {
        /* TODO: implement selection of special games
         * i.e. grand, null */
        app->trump = get_best_suit(app->re->cards);
    }

    /* print game to label */
    switch (app->trump)
    {
        case -1:
            g_sprintf(gamename, "Null");
            break;
        case KARO:
            g_sprintf(gamename, "Karo");
            break;
        case HERZ:
            g_sprintf(gamename, "Herz");
            break;
        case PIK:
            g_sprintf(gamename, "Pik");
            break;
        case KREUZ:
            g_sprintf(gamename, "Kreuz");
            break;
        case 0:
            g_sprintf(gamename, "Grand");
            break;
    }

    gtk_widget_set_sensitive(app->allwidgets[1], FALSE);
    gtk_label_set_text(GTK_LABEL(app->allwidgets[4]), gamename);

    /* hide skat & redraw screen */
    for (list = g_list_first(app->skat); list; list = list->next)
    {
        card = list->data;
        card->draw = FALSE;
    }

    /* reorder re player's cards */
    app->re->cards = g_list_sort_with_data(app->re->cards, compare_cards, app);

    /* reorder player's cards if necessary */
    if (app->re != app->players[0])
        app->players[0]->cards = g_list_sort_with_data(app->players[0]->cards,
                compare_cards, app);

    calc_card_positions(app);
    draw_area(app);

    play_stich(app);
}

void throw_card(app *app, card *card)
{
    player *player = app->players[card->owner];

    card->draw = TRUE;
    card->draw_face = TRUE;

#ifdef DEBUG
    g_print("%s played: ", player->name);
    print_card(card);
    g_print("\n");
#endif

    app->table = g_list_append(app->table, card);
    app->played = g_list_append(app->played, card);
    player->cards = g_list_remove(player->cards, card);

    calc_card_positions(app);
    draw_area(app);
}

void ai_play_card(app *app, player *player)
{
    GList *ptr = NULL;
    card *card = NULL;

    ptr = get_possible_cards(app, player->cards);

    /* TODO: implement AI logic here */
    card = ai_select_card(app, player, ptr);

    g_list_free(ptr);

    throw_card(app, card);
}

void calculate_stich(app *app)
{
    gchar msg[6];
    gint i, winner;
    gint points = 0;
    GList *ptr = NULL;
    card *crd = NULL;

    /* calculate winner of stich */
    if (is_greater(g_list_nth_data(app->table, 1),
                g_list_nth_data(app->table, 0), app->trump, app->null))
    {
        if (is_greater(g_list_nth_data(app->table, 2),
                    g_list_nth_data(app->table, 1), app->trump, app->null))
        {
            crd = g_list_nth_data(app->table, 2);
            winner = crd->owner;
        }
        else
        {
            crd = g_list_nth_data(app->table, 1);
            winner = crd->owner;
        }
    }
    else
    {
        if (is_greater(g_list_nth_data(app->table, 2),
                    g_list_nth_data(app->table, 0), app->trump, app->null))
        {
            crd = g_list_nth_data(app->table, 2);
            winner = crd->owner;
        }
        else
        {
            crd = g_list_nth_data(app->table, 0);
            winner = crd->owner;
        }
    }

    /* calculate points of stich */
    for (ptr = g_list_first(app->table); ptr; ptr = ptr->next)
    {
        crd = ptr->data;
        points += crd->points;

        crd->draw = FALSE;
    }

    app->players[winner]->points += points;
    app->player = winner;

    DPRINT(("%s won the stich (%d).\n", app->player_names[winner], points));

    /* add played cards to 'stiche' array */
    app->stiche[app->stich-1] = (card **) g_malloc(sizeof(card *) * 3);
    for (i=0; i<3; ++i)
        app->stiche[app->stich-1][i] = g_list_nth_data(app->table, i);

    /* remove cards from table */
    g_list_free(app->table);
    app->table = NULL;

    ++app->stich;

    /* update interface */
    if (app->stich <= 10)
    {
        g_sprintf(msg, "%d", app->stich);
        gtk_label_set_text(GTK_LABEL(app->allwidgets[2]), msg);
    }

    calc_card_positions(app);
    draw_area(app);
}

void end_round(app *app)
{
    gint rank, game;
    gchar msg[200];
    GList *list = NULL, *ptr = NULL;
    card *card = NULL;
    player *player = app->re;

    app->round += 1;

    if (!app->null)
    {
        switch (app->trump)
        {
            case KARO:
                rank = 9;
                break;
            case HERZ:
                rank = 10;
                break;
            case PIK:
                rank = 11;
                break;
            case KREUZ:
                rank = 12;
                break;
            case 0:
                rank = 24;
                break;
        }

        /* get player's cards */
        for (ptr = g_list_first(app->cards); ptr; ptr = ptr->next)
        {
            card = ptr->data;

            if (card->owner == player->id)
            {
                list = g_list_prepend(list, card);
#ifdef DEBUG
                print_card(card);
#endif
            }
        }

        /* add points of cards in skat */
        for (ptr = g_list_first(app->skat); ptr; ptr = ptr->next)
        {
            card = ptr->data;

            player->points += card->points;
        }

        game = get_spitzen(app, list, app->trump);

        /* hand game */
        if (app->hand)
            game += 1;

        g_list_free(list);

        /* player has won */
        if (player->points > 60)
        {
            /* player won 'schwarz' */
            if (player->points == 120)
                game += 2;
            /* player won 'schneider' */
            else if (player->points >= 90)
                game += 1;

            game *= rank;

            if (player->gereizt > game)
            {
                g_sprintf(msg, "%s hat ueberreizt.\nGereizt: %d\n"
                        "Spielwert: %d\n\t%d",
                        player->name,
                        player->gereizt,
                        game,
                        game * -2);

                game *= -2;
            }
            else
            {
                g_sprintf(msg, "%s gewinnt mit %d zu %d Punkten\n\t+%d",
                        player->name,
                        player->points,
                        (120 - player->points),
                        game);
            }
        }
        /* player has lost */
        else
        {
            /* player lost 'schwarz' */
            if (player->points == 0)
                game += 2;
            /* player lost 'schneider' */
            else if (player->points <= 30)
                game += 1;

            game = game * rank * -2;

            g_sprintf(msg, "%s verliert mit %d zu %d Punkten\n\t%d",
                    player->name,
                    player->points,
                    (120 - player->points),
                    game);
        }

        player->sum_points += game;

        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(app->allwidgets[0]),
                GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_INFO,
                GTK_BUTTONS_CLOSE,
                NULL);

        gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), msg);

        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
    else
    {
        /* TODO: implement null game */
        game = 23;
    }

    /* update interface */
    g_sprintf(msg, "%d", app->players[0]->sum_points);
    gtk_label_set_text(GTK_LABEL(app->allwidgets[6]), msg);
    g_sprintf(msg, "%d", app->players[1]->sum_points);
    gtk_label_set_text(GTK_LABEL(app->allwidgets[7]), msg);
    g_sprintf(msg, "%d", app->players[2]->sum_points);
    gtk_label_set_text(GTK_LABEL(app->allwidgets[8]), msg);

    g_sprintf(msg, "Runde %d", app->round);
    gtk_frame_set_label(GTK_FRAME(app->allwidgets[9]), msg);

    reset_game(app);
}

void play_stich(app *app)
{
    gint fh = (app->player == -1) ? app->forehand : app->player;
    gint num_cards = (app->table) ? g_list_length(app->table) : 0;
    gint player;

    player = (fh + num_cards) % 3;

    if (app->stich <= 10)
    {
        if (num_cards < 3)
        {
            if (!app->players[player]->human)
            {
                ai_play_card(app, app->players[player]);
                play_stich(app);
            }
        }
        else
        {
            /* wait for the user to press button */
            app->state = READY;
        }
    }
    else
        end_round(app);
}

void reset_game(app *app)
{
    gint i;
    GList *ptr = NULL;
    card *card = NULL;

    app->stich = 1;
    app->re = NULL;
    app->forehand = (app->forehand + 1) % 3;
    app->state = ENDGAME;
    app->player = app->forehand;
    app->trump = -1;
    app->hand = FALSE;

    /* remove cards from players if necessary */
    if (app->players)
    {
        for (i=0; i<3; ++i)
        {
            if (app->players[i]->cards)
                g_list_free(app->players[i]->cards);
            app->players[i]->cards = NULL;
            app->players[i]->re = FALSE;
            app->players[i]->gereizt = 0;
            app->players[i]->points = 0;
        }
    }

    /* empty played stiche if necessary */
    if (app->stiche)
    {
        for (i=0; i<10; ++i)
        {
            if (app->stiche[i])
                g_free(app->stiche[i]);
            app->stiche[i] = NULL;
        }
    }

    /* empty played cards if necessary */
    if (app->played)
        g_list_free(app->played);
    app->played = NULL;

    /* empty skat if necessary */
    if (app->skat)
        g_list_free(app->skat);
    app->skat = NULL;

    /* empty table if necessary */
    if (app->table)
        g_list_free(app->table);
    app->table = NULL;

    /* reset all cards */
    for (ptr = g_list_first(app->cards); ptr; ptr = ptr->next)
    {
        card = ptr->data;

        card->owner = -1;
        card->draw = FALSE;
        card->draw_face = FALSE;
        card->status = CS_AVAILABLE;
    }

    /* update interface */
    gtk_label_set_text(GTK_LABEL(app->allwidgets[2]), "1");
    gtk_label_set_text(GTK_LABEL(app->allwidgets[3]), "-");
    gtk_label_set_text(GTK_LABEL(app->allwidgets[4]), "-");
    gtk_label_set_text(GTK_LABEL(app->allwidgets[5]), "-");
    gtk_widget_set_sensitive(app->allwidgets[1], TRUE);
    gtk_button_set_label(GTK_BUTTON(app->allwidgets[1]), "Neue Runde");
}

void game_start(app *app)
{
    /* give cards */
    app->state = GIVE_CARDS;
    give_cards(app);

    /* recalculate card positions */
    calc_card_positions(app);

    /* refresh screen */
    draw_area(app);

    app->state = WAITING;
    app->re = NULL;

    /* activate button */
    gtk_widget_set_sensitive(app->allwidgets[1], TRUE);
}

/* vim:set et sw=4 ts=4 tw=80: */
