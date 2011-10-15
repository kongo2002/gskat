/*
 *  This file is part of gskat.
 *
 *  Copyright (C) 2010-2011 by Gregor Uhlenheuer
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
#include "callback.h"
#include "common.h"
#include "configuration.h"
#include "draw.h"
#include "game.h"
#include "interface.h"
#include "utils.h"

/**
 * card_to_player:
 * @player:  pointer to #player that gets the @card
 * @card:    pointer to #card to give to @player
 *
 * Give a #card to @player
 */
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

/**
 * give_to_skat:
 * @card:  #card to give into skat
 *
 * Give a card into skat
 */
void give_to_skat(card *card)
{
    card->draw = TRUE;
    card->draw_face = FALSE;
    card->status = CS_DISTRIBUTED;
    card->owner = -1;

    gskat.skat = g_list_prepend(gskat.skat, (gpointer) card);
}

/**
 * play_card:
 * @event:  #GdkEventButton structure
 *
 * Play the card the user clicked on
 *
 * It's checked if the clicked card is possible to play
 * and if it's the player's turn to play.
 *
 * Returns: %TRUE if the card was played, else %FALSE
 */
gboolean play_card(GdkEventButton *event)
{
    gint num_cards = (gskat.table) ? g_list_length(gskat.table) : 0;
    GList *ptr = NULL;
    card *card = NULL;
    player *player = gskat.players[0];

    /* check if it's the player's turn */
    if (((gskat.cplayer + num_cards) % 3) == 0)
    {
        /* get card that has been clicked on */
        if ((card = click_card(event, player->cards)))
        {
            /* check if card is possible */
            ptr = get_possible_cards(player->cards);
            if (g_list_index(ptr, card) != -1)
            {
                throw_card((gpointer) card);

                g_list_free(ptr);

                return TRUE;
            }
            else
                gskat_msg(MT_DEBUG, _("Card is not possible.\n"));

            if (ptr)
                g_list_free(ptr);
        }
    }

    return FALSE;
}

/**
 * click_skat:
 * @event:  #GdkEventButton structure
 *
 * Swaps the clicked card with one of the cards in skat
 *
 * Returns: %TRUE when the cards could be swapped, otherwise %FALSE
 */
gboolean click_skat(GdkEventButton *event)
{
    player *player = gskat.players[0];
    card *card = NULL;

    if ((card = click_card(event, player->cards)))
    {
        /* swap clicked card with last card in skat */
        gskat.skat = g_list_prepend(gskat.skat, (gpointer) card);
        player->cards = g_list_remove(player->cards, (gconstpointer) card);

        card = g_list_nth_data(gskat.skat, 2);
        player->cards = g_list_prepend(player->cards, (gpointer) card);
        gskat.skat = g_list_remove(gskat.skat, (gconstpointer) card);

        player->cards = g_list_sort(player->cards, compare_cards);

        /* redraw screen */
        calc_card_positions();
        draw_area();

        return TRUE;
    }

    return FALSE;
}

/**
 * click_card:
 * @event:  #GdkEventButton structure
 * @list:   #GList of possible cards to choose from
 *
 * Gets the card the player clicked on
 *
 * Returns: pointer to the #card clicked on or %NULL if at the position
 * the user clicked on there is not #card
 */
card *click_card(GdkEventButton *event, GList *list)
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
            return card;
        }
    }
    return NULL;
}

/**
 * give_cards:
 *
 * Distribute the 32 cards in the deck to the players/skat
 *
 * Every player gets 10 cards whereas the last 2 cards go into
 * the skat in the middle of the table.
 */
void give_cards(void)
{
    gint order[32];
    gint i, j, k;
    player *player;

    /* init order list */
    for (i=0; i<32; ++i)
        order[i] = i;

    /* generate random card order */
    for (i=0; i<100; ++i)
    {
        j = g_random_int_range(0, 32);
        do
            k = g_random_int_range(0, 32);
        while (k == j);
        swap(&order[j], &order[k]);
    }

    /* give cards to players */
    for (i=0; i<3; ++i)
    {
        player = gskat.players[i];
        for (j=0; j<10; ++j)
        {
            card_to_player(player,
                    g_list_nth_data(gskat.cards, order[j+(i*10)]));
        }
        player->cards = g_list_sort(player->cards, compare_cards);
    }

    /* two cards for skat */
    give_to_skat(g_list_nth_data(gskat.cards, order[30]));
    give_to_skat(g_list_nth_data(gskat.cards, order[31]));
}

/**
 * get_bid_response:
 * @value:   bid value
 * @msg:     string containing the title of the message box
 * @hoeren:  user has to 'hear'?
 *
 * Gets a bid response from the user by opening a message box
 *
 * When the user has to 'hear' he gets the possible answers 'Yes'
 * and 'No' otherwise the next bid value and 'Pass'.
 *
 * Returns: If the user continues to bid that value is returned.
 * Otherwise 0 is returned representing a 'pass'.
 */
void get_bid_response(gint value, gchar *msg, gboolean hoeren)
{
    gint result;
    gchar caption[4];
    g_sprintf(caption, "%d", value);

    GtkWidget *dialog = gtk_dialog_new_with_buttons(_("Bidding"),
            GTK_WINDOW(gskat.window),
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            (hoeren) ? _("Yes") : caption, value,
            (hoeren) ? _("No") : _("Pass"), 0,
            NULL);

    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
            gtk_label_new(msg), FALSE, TRUE, 2);
    gtk_widget_show_all(dialog);

    result = gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);

    do_player_bid(result, hoeren);
}

/**
 * next_reizwert:
 * @value: current bid value
 *
 * Returns: the next value to bid
 */
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

/**
 * get_possible_cards:
 * @list:  #GList of cards to search in for possible cards
 *
 * Return a #GList of cards that are possible to play
 * based on the cards currently on the table
 *
 * Returns: (transfer full): a new #GList with all cards that are possible
 * or allowed to play. If no #card is possible or allowed return %NULL.
 */
GList *get_possible_cards(GList *list)
{
    GList *ptr = NULL;
    card *card = NULL;

    /* all cards are possible if there is no card on the table
     * copy the list because the returned list will be freed after usage */
    if (gskat.table == NULL || g_list_length(gskat.table) == 0)
        return g_list_copy(list);
    else
    {
        card = g_list_nth_data(gskat.table, 0);

        if (is_trump(card))
            ptr = get_trump_list(list);
        else
            ptr = get_suit_list(list, card->suit);

        if (ptr)
            return ptr;
        else
            return g_list_copy(list);
    }
    return NULL;
}

/**
 * rate_cards:
 * @player:  #player to calculate the cards for
 * @list:    #GList of the player's cards
 *
 * Returns a rating of a given card deck
 *
 * This function does a simple calculation of the rating of a
 * player's card deck. Currently the longest suit, the number of
 * jacks and suits you can trump are analyzed.
 *
 * Returns: card deck rating
 */
gint rate_cards(player *player, GList *list)
{
    gint i, jacks = 0, rate = 0, best = 0;
    GList *ptr = NULL;
    card *card = NULL;

    best = get_best_suit(list);

    /* +1 if forehand */
    if (gskat.forehand == player->id)
        rate++;

    /* no or one card of one suit? */
    for (i=0; i<4; ++i)
    {
        ptr = get_suit_list(list, SUITS[i]);

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

/**
 * get_best_suit:
 * @list: #GList of cards to search in
 *
 * Find the best suit in a list of cards
 *
 * Returns: index of the selected (best) suit
 */
gint get_best_suit(GList *list)
{
    gint i, max = 0, ret = 0;
    gint rank_count[4];
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

/**
 * get_max_reizwert:
 * @list:  #GList of cards to analyze
 *
 * Returns the maximum value to bid
 *
 * Returns: maximum value to bid
 */
gint get_max_reizwert(GList *list)
{
    gint suit, max;

    g_return_val_if_fail(list, 0);

    suit = get_best_suit(list);
    max = get_spitzen(list, suit, NULL);

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

/**
 * do_player_bid:
 * @response: Player's response
 * @hoeren:   Is the player on the hearing position?
 *
 * Execute the player's bidding process
 */
void do_player_bid(gint response, gboolean hoeren)
{
    player *self = gskat.players[0];

    if (response)
    {
        self->gereizt = response;
        gskat.bidden = response;

        if (hoeren)
        {
            /* last call */
            if (!gskat.bidden)
                start_bidding();

            do_sagen(gskat.players[gskat.sager], gskat.hoerer,
                    next_reizwert(response));
        }
        else
            do_hoeren(gskat.players[gskat.hoerer], response, self->id);
    }
    else
    {
        self->gereizt = -1;

        if (hoeren)
            gskat.hoerer = gskat.sager;

        gskat.sager = (gskat.forehand + 2) % 3;
        start_bidding();
    }
}

/**
 * do_hoeren:
 * @player:  hearing player
 * @value:   bid value that is asked
 * @sager:   saying player's index
 *
 * Execute the 'hearing' process for the given player
 */
void do_hoeren(player *player, gint value, gint sager)
{
    gint max = 0;
    gchar *msg;

    if (!player->human)
    {
        max = get_max_reizwert(player->cards);

        if (sager)
            g_usleep(G_USEC_PER_SEC);

        player->does_bid = TRUE;

        /* draw player's bid on-screen if configured */
        if (get_prop_bool("provoke_values"))
            g_timeout_add(3000, (GSourceFunc) player_draw_bid,
                    (gpointer) player);

        if (rate_cards(player, player->cards) >= 7 && value <= max)
        {
            gskat_msg(MT_DEBUG | MT_BUGREPORT,
                    _("%s says 'yes' to %d\n"), player->name, value);

            player->gereizt = value;
            gskat.hoerer = player->id;

            draw_area();

            do_sagen(gskat.players[sager], player->id, next_reizwert(value));
        }
        else
        {
            gskat_msg(MT_DEBUG | MT_BUGREPORT,
                    _("%s passes at %d\n"), player->name, value);

            player->gereizt = -1;
            gskat.sager = (gskat.forehand + 2) % 3;
            gskat.hoerer = sager;

            start_bidding();
        }
    }
    else
    {
        msg = g_strdup_printf(_("%s says %d. Do you accept?"),
                gskat.players[sager]->name, value);

#if GTK_CHECK_VERSION(2, 18, 0)
        show_bid_infobar(value, msg, TRUE);
#else
        get_bid_response(value, msg, TRUE);
#endif

        g_free(msg);
    }
}

/**
 * do_last_call:
 *
 * Ask the last remaining player if he wants to play lead the game
 * as Re player. The two other players have passed beforehand.
 */
void do_last_call(void)
{
    gint i, max;
    player *rem = NULL;

    /* determine remaining player to ask */
    for (i=0; i<3; ++i)
    {
        if (gskat.players[i]->gereizt != -1)
        {
            gskat.hoerer = i;
            rem = gskat.players[i];
            break;
        }
    }

    if (!rem->human)
    {
        max = get_max_reizwert(rem->cards);

        if (rate_cards(rem, rem->cards) >= 7 && 18 <= max)
        {
            gskat_msg(MT_DEBUG | MT_BUGREPORT, _("%s says 18\n"), rem->name);

            rem->gereizt = 18;
            gskat.bidden = 18;
        }

        start_bidding();
    }
    else
#if GTK_CHECK_VERSION(2, 18, 0)
        show_bid_infobar(18, _("Do you want to bid 18?"), TRUE);
#else
        get_bid_response(18, _("Do you want to bid 18?"), TRUE);
#endif
}

/**
 * do_sagen:
 * @player:  saying player
 * @hoerer:  hearing player's index
 * @value:   last bidden value
 *
 * Execute the 'saying' process for the given player
 */
void do_sagen(player *player, gint hoerer, gint value)
{
    gint max = 0;
    gchar *msg;

    gskat_msg(MT_DEBUG | MT_BUGREPORT,
            _("do_sagen(): %s asks %s\n"), player->name,
            gskat.players[hoerer]->name);

    if (!player->human)
    {
        max = get_max_reizwert(player->cards);

        /* draw player's bid on-screen */
        player->does_bid = TRUE;
        g_timeout_add(3000, (GSourceFunc) player_draw_bid, (gpointer) player);

        /* check if player wants to bid (further) */
        if (rate_cards(player, player->cards) >= 7 && value <= max)
        {
            gskat_msg(MT_INFO | MT_BUGREPORT,
                    _("%s says %d\n"), player->name, value);

            player->gereizt = value;
            gskat.sager = player->id;
            gskat.bidden = value;

            draw_area();

            /* ask the hearing player */
            do_hoeren(gskat.players[hoerer], value, player->id);
        }
        /* player passes */
        else
        {
            gskat_msg(MT_DEBUG | MT_BUGREPORT,
                    _("%s passes at %d\n"), player->name, value);

            player->gereizt = -1;
            gskat.sager = (gskat.forehand + 2) % 3;
            gskat.hoerer = hoerer;

            return start_bidding();
        }
    }
    else
    {
        msg = g_strdup_printf(_("Do you want to bid %d?"), value);

#if GTK_CHECK_VERSION(2, 18, 0)
        show_bid_infobar(value, msg, FALSE);
#else
        get_bid_response(value, msg, FALSE);
#endif

        g_free(msg);
    }
}

/**
 * start_bidding:
 *
 * Initialize bidding process
 */
void start_bidding(void)
{
    gint i;
    player *pptr;

    /* refresh game area */
    draw_area();

    /* first bidding phase */
    if (gskat.state == PROVOKE1)
    {
        gskat.state = PROVOKE2;
        gskat.hoerer = gskat.forehand;
        gskat.sager = (gskat.hoerer + 1) % 3;
        gskat.bidden = 0;

        /* disable 'new round' button */
        gtk_widget_set_sensitive(get_widget("button"), FALSE);

        /* reset all player values */
        for (i=0; i<3; ++i)
        {
            pptr = gskat.players[i];
            pptr->gereizt = 0;

            gskat_msg(MT_DEBUG | MT_BUGREPORT,
                    _("MaxReizwert of %s: %d\n"), pptr->name,
                    get_max_reizwert(pptr->cards));
            gskat_msg(MT_DEBUG | MT_BUGREPORT,
                    _("CardRating of %s: %d\n"), pptr->name,
                    rate_cards(pptr, pptr->cards));
        }

        do_sagen(gskat.players[gskat.sager], gskat.hoerer, 18);
    }
    /* second bidding phase */
    else if (gskat.state == PROVOKE2)
    {
        gskat.state = PROVOKE3;

        do_sagen(gskat.players[gskat.sager], gskat.hoerer,
                (gskat.bidden) ? gskat.bidden : 18);
    }
    else if (gskat.state == PROVOKE3)
    {
        gskat.state = PROVOKE4;

        /* first two players have passed */
        if (!gskat.bidden)
            do_last_call();
        else
            start_bidding();
    }
    else if (gskat.state == PROVOKE4)
    {
        if (gskat.bidden)
        {
            gskat.re = (gskat.players[gskat.sager]->gereizt >
                    gskat.players[gskat.hoerer]->gereizt) ?
                gskat.players[gskat.sager] : gskat.players[gskat.hoerer];

            gskat.re->re = TRUE;

            /* update interface */
            gtk_widget_set_sensitive(get_widget("button"), TRUE);
            gtk_button_set_label(GTK_BUTTON(get_widget("button")),
                    _("Pronounce a game"));

            update_interface();
            take_skat();
        }
        else
        {
            gskat_msg(MT_INFO | MT_DIALOG | MT_BUGREPORT,
                    _("All players have passed.\nNew round.\n"));

            reset_game();
            game_start();
        }
    }
}

/**
 * druecke_skat:
 *
 * Select cards to be put in skat
 */
void druecke_skat(void)
{
    gint suits[] = { 40, 60, 80, 100 };
    gint count = 0;
    gint i, best;
    GList *ptr = NULL;
    card *card = NULL;
    player *player = gskat.re;

    /* add skat to player's cards */
    player->cards = g_list_prepend(player->cards,
            g_list_nth_data(gskat.skat, 0));
    player->cards = g_list_prepend(player->cards,
            g_list_nth_data(gskat.skat, 1));

    /* empty skat */
    g_list_free(gskat.skat);
    gskat.skat = NULL;

    best = get_best_suit(player->cards);

    /* search for a suit with only 1 card left
     * when this card is no ASS -> take it */
    for (i=0; i<4; ++i)
    {
        if (suits[i] != best && count < 2)
        {
            ptr = get_suit_list(player->cards, suits[i]);

            if (g_list_length(ptr) == 1)
            {
                card = g_list_nth_data(ptr, 0);
                if (card->rank != ASS)
                {
                    gskat.skat = g_list_prepend(gskat.skat, card);
                    player->cards = g_list_remove(player->cards, card);
                    ++count;
                }
            }

            g_list_free(ptr);
            ptr = NULL;
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
                if (ptr)
                {
                    g_list_free(ptr);
                    ptr = NULL;
                }

                ptr = get_suit_list(player->cards, suits[i]);

                if (g_list_length(ptr) == 2)
                {
                    card = g_list_nth_data(ptr, 0);
                    if (card->rank == ASS)
                        continue;

                    gskat.skat = g_list_prepend(gskat.skat, card);
                    player->cards = g_list_remove(player->cards, card);

                    card = g_list_nth_data(ptr, 1);
                    gskat.skat = g_list_prepend(gskat.skat, card);
                    player->cards = g_list_remove(player->cards, card);

                    count = 2;
                    break;
                }
            }
        }

        if (ptr)
        {
            g_list_free(ptr);
            ptr = NULL;
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
                ptr = get_suit_list(player->cards, suits[i]);

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

        gskat.skat = g_list_prepend(gskat.skat, card);
        player->cards = g_list_remove(player->cards, card);
        ++count;

        if (minimum)
            g_list_free(minimum);
    }

#ifdef DEBUG
    print_card(g_list_nth_data(gskat.skat, 0)); g_print("\n");
    print_card(g_list_nth_data(gskat.skat, 1)); g_print("\n");
#endif
}

/**
 * take_skat:
 *
 * Take the two cards in the skat or play 'hand'
 */
void take_skat(void)
{
    gint result;
    GList *ptr = NULL;
    card *card = NULL;

    if (gskat.re->human)
    {
        do
        {
            GtkWidget *dialog = gtk_dialog_new_with_buttons(_("Play hand game?"),
                    GTK_WINDOW(gskat.window),
                    GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
                    _("Yes"), 1,
                    _("No"), 0,
                    NULL);

            gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
                    gtk_label_new(_("Play hand game?")), FALSE, TRUE, 2);
            gtk_widget_show_all(dialog);

            result = gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }
        while (result == GTK_RESPONSE_DELETE_EVENT);

        if (result == 1)
            gskat.hand = TRUE;
    }

    gskat.state = TAKESKAT;

    for (ptr = g_list_first(gskat.skat); ptr; ptr = ptr->next)
    {
        /* update card owner */
        card = ptr->data;
        card->owner = gskat.re->id;

        /* show cards in skat */
        if (gskat.re->human && !gskat.hand)
        {
            card->draw = TRUE;
            card->draw_face = TRUE;
        }
    }

    if (!gskat.re->human)
    {
        druecke_skat();
        spiel_ansagen();

        gskat.state = PLAYING;
    }

    /* update screen */
    draw_area();
}

/**
 * spiel_ansagen:
 *
 * Decide and say what game type (trump) should be played this round
 */
void spiel_ansagen(void)
{
    gint i, result = 0;
    GList *list = NULL;
    GtkWidget *dialog;
    GtkWidget *button;
    card *card = NULL;

    gskat_msg(MT_INFO | MT_BUGREPORT, _("Pronounce game.\n"));

    /* select game to play */
    if (gskat.re->human)
    {
        /* TODO: we want to have a confirm dialog instead
         * of forcing the user to click on something valid */
        do
        {
            dialog = gtk_dialog_new();

            /* add buttons */
            for (i=0; i<4; ++i)
            {
                button = gtk_dialog_add_button(GTK_DIALOG(dialog),
                        suit_name(SUITS[i]), SUITS[i]);

                /* add suit images if available */
                if (gskat.icons[i])
                    gtk_button_set_image(GTK_BUTTON(button),
                            gtk_image_new_from_pixbuf(gskat.icons[i]));
            }

            gtk_dialog_add_button(GTK_DIALOG(dialog), "Grand", 200);
            gtk_dialog_add_button(GTK_DIALOG(dialog), "Null", 0);

            result = gtk_dialog_run(GTK_DIALOG(dialog));

            gtk_widget_destroy(dialog);
        }
        while (result == GTK_RESPONSE_DELETE_EVENT);

        /* set game/trump */
        if (result == 200)
            gskat.trump = 0;
        else if (result == 0)
        {
            gskat.trump = -1;
            gskat.null = TRUE;
        }
        else
            gskat.trump = result;
    }
    else
    {
        /* TODO: implement selection of special games
         * i.e. grand, null */
        gskat.trump = get_best_suit(gskat.re->cards);

        dialog = gtk_message_dialog_new(GTK_WINDOW(gskat.window),
                GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_INFO,
                GTK_BUTTONS_CLOSE,
                _("%s plays %s."), gskat.re->name, suit_name(gskat.trump));

        gtk_dialog_run(GTK_DIALOG(dialog));

        gtk_widget_destroy(dialog);
    }

    /* update statusbar */
    gskat_msg(MT_STATUSBAR, _("%s plays %s"), gskat.re->name,
            suit_name(gskat.trump));

    /* print game to label */
    update_interface();

    gtk_widget_set_sensitive(get_widget("button"), FALSE);
    gtk_widget_set_sensitive(get_widget("mi_quicksave"), TRUE);
    gtk_widget_set_sensitive(get_widget("mi_bugreport"), TRUE);
    gtk_widget_set_sensitive(get_widget("mi_gamesave"), TRUE);

    /* hide skat & redraw screen */
    for (list = g_list_first(gskat.skat); list; list = list->next)
    {
        card = list->data;
        card->draw = FALSE;
    }

    /* reorder re player's cards */
    gskat.re->cards = g_list_sort(gskat.re->cards, compare_cards);

    /* reorder player's cards if necessary */
    if (gskat.re != gskat.players[0])
        gskat.players[0]->cards = g_list_sort(gskat.players[0]->cards,
                compare_cards);

    calc_card_positions();
    draw_area();

    play_stich();
}

trick *trick_new(void)
{
    trick *tmp = (trick *) g_malloc(sizeof(trick));
    card **cards = (card **) g_malloc0(sizeof(card *) * 3);

    tmp->cards = cards;
    tmp->points = 0;
    tmp->winner = NULL;

    return tmp;
}

/**
 * throw_card:
 * @data:  #card to throw on the table (cast to #gpointer)
 *
 * Throw given card on the table
 *
 * Returns: %FALSE in order to destroy the delay timeout
 */
gboolean throw_card(gpointer data)
{
    gint index;
    card *_card = (card *) data;
    trick *trick = gskat.stiche[gskat.stich-1];
    card_move *cm = NULL;

    player *player = gskat.players[_card->owner];

    _card->draw = TRUE;
    _card->draw_face = TRUE;

    gskat_msg(MT_INFO | MT_BUGREPORT,
            _("%s played: %s\n"), player->name, get_card_name(_card));

    gskat.table = g_list_append(gskat.table, _card);
    gskat.played = g_list_append(gskat.played, _card);
    player->cards = g_list_remove(player->cards, _card);

    /* keep record of played cards */
    if (trick == NULL)
    {
        trick = trick_new();
        gskat.stiche[gskat.stich-1] = trick;
    }

    index = g_list_length(gskat.table) - 1;
    trick->cards[index] = _card;

    if (get_prop_bool("animation"))
    {
        /* initiate card movement animation */
        _card->status = CS_MOVING;

        /* this will be freed in the last call of 'move_card' */
        cm = (card_move *) g_malloc(sizeof(card_move));

        cm->mcard = _card;
        set_table_position(_card, &cm->dest_x, &cm->dest_y);
        set_card_move_step(cm);

        g_timeout_add(25, (GSourceFunc) move_card, (gpointer) cm);
    }

    calc_card_positions();
    draw_area();

    /* trigger next action
     * either next card or new trick */
    play_stich();

    return FALSE;
}

/**
 * ai_play_card:
 * @player:  pointer to AI #player
 *
 * Trigger AI to play a card
 */
void ai_play_card(player *player)
{
    GList *ptr = NULL;
    card *card = NULL;

    ptr = get_possible_cards(player->cards);

    card = ai_select_card(player, ptr);

    g_list_free(ptr);

    /* delay the card throw if configured */
    if (get_prop_bool("reaction") && get_prop_int("reaction_duration") > 0)
        g_timeout_add(get_prop_int("reaction_duration"), (GSourceFunc) throw_card,
                (gpointer) card);
    else
        throw_card((gpointer) card);
}

/**
 * calculate_stich:
 *
 * Calculate the winner and the points of the last played trick
 */
void calculate_stich(void)
{
    gint winner;
    gint points = 0;
    GList *ptr = NULL;
    card *crd = NULL;

    winner = get_table_winner();

    gskat_msg(MT_STATUSBAR, _("%s won the trick."),
            gskat.players[winner]->name);

    /* calculate points of stich */
    for (ptr = g_list_first(gskat.table); ptr; ptr = ptr->next)
    {
        crd = ptr->data;
        points += crd->points;

        crd->draw = FALSE;
    }

    gskat.players[winner]->points += points;
    gskat.cplayer = winner;

    /* check if a 'null' playing re player won the stich
     * -> he would have lost the game then */
    if (gskat.re == gskat.players[winner] && gskat.null)
    {
        gskat_msg(MT_INFO | MT_BUGREPORT,
                _("%s lost the game (null game)\n"), gskat.re->name);

        return end_round(FT_LOST);
    }

    gskat_msg(MT_INFO | MT_BUGREPORT,
            _("%s won the trick (%d).\n"), gskat.players[winner]->name, points);

    /* remove cards from table */
    g_list_free(gskat.table);
    gskat.table = NULL;

    ++gskat.stich;

    /* update interface */
    if (gskat.stich <= 10)
        update_interface();

    calc_card_positions();
    draw_area();
}

/**
 * set_round_points:
 * @winner:  Id of the #player that won the round
 * @points:  Game points of the round
 *
 * Refresh the points each #player got in the played round
 */
void set_round_points(gint winner, gint points)
{
    gint i;
    player *cur = NULL;

    for (i=0; i<3; ++i)
    {
        cur = gskat.players[i];

        if (i == winner)
        {
            cur->sum_points += points;
            cur->round_points = g_list_append(cur->round_points,
                    GINT_TO_POINTER(points));
        }
        else
            cur->round_points = g_list_append(cur->round_points,
                    GINT_TO_POINTER(0));
    }
}

/**
 * end_round:
 * @ft: the #finish_type of the ending of the current round
 *
 * Finalize the last round by refreshing all settings
 * and player's points
 */
void end_round(finish_type ft)
{
    gint rank, game;
    GtkTreeStore *tree;
    GtkWidget *sum;
    GList *ptr;
    card *card;
    player *player = gskat.re;

    /* increase round counter */
    gskat.round += 1;

    /* add points of cards in skat */
    for (ptr = g_list_first(gskat.skat); ptr; ptr = ptr->next)
    {
        card = ptr->data;
        player->points += card->points;
    }

    /* initialize summary window */
    sum = create_game_summary(&tree);

    /* determine game type multiplier */
    rank = get_game_multiplier();

    if (!gskat.null)
    {
        game = get_game_base_value(player, &tree);

        /* hand game */
        if (gskat.hand)
        {
            game += 1;
            add_summary_row_int(&tree, _("Hand"), game);
        }

        /* player won or lost 'schneider' */
        if (player->points >= 90 || player->points < 30)
        {
            game += 1;
            add_summary_row_int(&tree, _("Schneider"), game);
        }

        /* player won or lost'schwarz' */
        if (player->points == 120 || player->points == 0)
        {
            game += 1;
            add_summary_row_int(&tree, _("Schwarz"), game);
        }

        add_summary_row_int(&tree, get_game_name(), rank);
        game *= rank;

        /* player has overbid */
        if (player->gereizt > game)
        {
            game *= -2;
            add_summary_row_int(&tree, _("Overbid"), game);
        }
        /* player has lost */
        else if (player->points <= 60)
        {
            game *= -2;
            add_summary_row_int(&tree, _("Lost"), game);
        }
        /* player has won */
        else
            add_summary_row_int(&tree, _("Won"), game);
    }
    /* null game */
    else
    {
        if (gskat.hand)
        {
            game = 35;
            add_summary_row_int(&tree, _("Null hand"), game);
        }
        else
        {
            game = 23;
            add_summary_row_int(&tree, _("Null"), game);
        }

        /* player has lost the 'null' game */
        if (ft == FT_LOST)
        {
            game *= -2;
            add_summary_row_int(&tree, _("Lost"), game);
        }
        /* player has overbid the 'null' game */
        else if (player->gereizt > game)
        {
            game *= -2;
            add_summary_row_int(&tree, _("Overbid"), game);
        }
        /* player has won the 'null' game */
        else
            add_summary_row_int(&tree, _("Won"), game);
    }

    /* show game summary dialog window */
    gtk_widget_show_all(sum);

    /* refresh the players' new points */
    set_round_points(player->id, game);

    /* update interface */
    update_rank_interface();
    update_interface();

    gtk_widget_set_sensitive(get_widget("mi_quicksave"), FALSE);
    gtk_widget_set_sensitive(get_widget("mi_bugreport"), FALSE);
    gtk_widget_set_sensitive(get_widget("mi_gamesave"), FALSE);

    /* reset game values */
    reset_game();

    calc_card_positions();
    draw_area();
}

/**
 * play_stich:
 *
 * Trigger the next player to play his card
 */
void play_stich(void)
{
    gint fh = (gskat.cplayer == -1) ? gskat.forehand : gskat.cplayer;
    gint num_cards = (gskat.table) ? g_list_length(gskat.table) : 0;
    gint current;

    current = (fh + num_cards) % 3;

    if (gskat.stich <= 10)
    {
        if (num_cards < 3)
        {
            if (!gskat.players[current]->human)
                ai_play_card(gskat.players[current]);
            else
                gskat_msg(MT_STATUSBAR, _("Choose a card to play"));
        }
        else
        {
            /* wait for the user to press button */
            gskat.state = READY;
        }
    }
    else
        end_round(FT_NORMAL);
}

/**
 * game_abort:
 *
 * Ask user if the user really wants to abort the current game round
 *
 * Returns: %TRUE if the user wants to abort the current round,
 * othwerise %FALSE
 */
gboolean game_abort(void)
{
    gboolean abort = FALSE;
    gint response;
    GtkWidget *dialog;

    dialog = gtk_message_dialog_new(GTK_WINDOW(gskat.window),
            GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_YES_NO,
            _("Really abort the current round?"));

    response = gtk_dialog_run(GTK_DIALOG(dialog));

    switch (response)
    {
        case GTK_RESPONSE_YES:
            gskat_msg(MT_STATUSBAR, _("Current round aborted."));
            abort = TRUE;
            break;
        case GTK_RESPONSE_NO:
            abort = FALSE;
            break;
        default:
            break;
    }

    gtk_widget_destroy(dialog);

    return abort;
}

/**
 * reset_game:
 *
 * Reset all game settings and free the allocated memory
 */
void reset_game(void)
{
    gint i;
    GList *ptr = NULL;
    card *card = NULL;

    gskat.stich = 1;
    gskat.re = NULL;
    gskat.forehand = (gskat.forehand + 1) % 3;
    gskat.state = ENDGAME;
    gskat.cplayer = gskat.forehand;
    gskat.trump = -1;
    gskat.hand = FALSE;
    gskat.null = FALSE;

    /* remove cards from players if necessary */
    if (gskat.players)
    {
        for (i=0; i<3; ++i)
        {
            if (gskat.players[i]->cards)
                g_list_free(gskat.players[i]->cards);
            gskat.players[i]->cards = NULL;
            gskat.players[i]->re = FALSE;
            gskat.players[i]->gereizt = 0;
            gskat.players[i]->points = 0;
        }
    }

    /* empty played stiche if necessary */
    if (gskat.stiche)
    {
        for (i=0; i<10; ++i)
        {
            if (gskat.stiche[i])
                g_free(gskat.stiche[i]);
            gskat.stiche[i] = NULL;
        }
    }

    /* empty played cards if necessary */
    if (gskat.played)
        g_list_free(gskat.played);
    gskat.played = NULL;

    /* empty skat if necessary */
    if (gskat.skat)
        g_list_free(gskat.skat);
    gskat.skat = NULL;

    /* empty table if necessary */
    if (gskat.table)
        g_list_free(gskat.table);
    gskat.table = NULL;

    /* reset all cards */
    for (ptr = g_list_first(gskat.cards); ptr; ptr = ptr->next)
    {
        card = ptr->data;

        card->owner = -1;
        card->draw = FALSE;
        card->draw_face = FALSE;
        card->status = CS_AVAILABLE;
    }

    /* reset bug report log */
    if (gskat.log)
        g_string_free(gskat.log, TRUE);
    gskat.log = g_string_sized_new(256);

    /* update interface */
    update_interface();
    gtk_widget_set_sensitive(get_widget("button"), TRUE);
    gtk_button_set_label(GTK_BUTTON(get_widget("button")), _("New round"));
}

/**
 * game_start:
 *
 * Distribute cards and start a new round
 */
void game_start(void)
{
    /* give cards */
    gskat.state = GIVE_CARDS;
    give_cards();

    /* recalculate card positions */
    calc_card_positions();

    /* refresh screen */
    draw_area();

    gskat.state = WAITING;
    gskat.re = NULL;

    /* activate button */
    gtk_widget_set_sensitive(get_widget("button"), TRUE);

    gskat_msg(MT_STATUSBAR, _("New round started"));
}

/* vim:set et sw=4 sts=4 tw=80: */
