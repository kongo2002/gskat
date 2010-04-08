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
#include "game.h"
#include "ai.h"
#include "interface.h"
#include "utils.h"

/**
 * @brief Give a card to player
 *
 * @param player  pointer to player that gets the card
 * @param card    pointer to card to give to player
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
 * @brief Give a card into skat
 *
 * @param app   main application object
 * @param card  card to give into skat
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
 * @brief Play the card clicked on
 *
 * It's checked if the clicked card is possible to play
 * and if it's the player's turn to play.
 *
 * @param app    main application object
 * @param event  button event pointer
 *
 * @return TRUE if the card was played, else FALSE
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
                throw_card(card);
                play_stich();
                return TRUE;
            }
            else
                DPRINT(("Card is not possible.\n"));
        }
    }

    return FALSE;
}

/**
 * @brief Swaps the clicked card with one of the cards in skat
 *
 * @param app    main application object
 * @param event  button event pointer
 *
 * @return TRUE when the cards could be swapped
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

        player->cards = g_list_sort_with_data(player->cards,
                compare_cards, &gskat);

        /* redraw screen */
        calc_card_positions();
        draw_area();

        return TRUE;
    }

    return FALSE;
}

/**
 * @brief Gets the card the player clicked on
 *
 * @param app    main application objects
 * @param event  button event pointer
 * @param list   list of possible cards to choose from
 *
 * @return pointer to the card clicked on or NULL if at the position
 * the user clicked on there is not card
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
            print_card(card);
            return card;
        }
    }
    return NULL;
}

/**
 * @brief Distribute the 32 cards in the deck to the players/skat
 *
 * Every player gets 10 cards whereas the last 2 cards go into
 * the skat in the middle of the table.
 *
 * @param app main application objects
 */
void give_cards()
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
        player = gskat.players[i];
        for (j=0; j<10; ++j)
        {
            card_to_player(player,
                    g_list_nth_data(gskat.cards, order[j+(i*10)]));
        }
        player->cards = g_list_sort_with_data(player->cards,
                compare_cards, &gskat);
    }

    /* two cards for skat */
    give_to_skat(g_list_nth_data(gskat.cards, order[30]));
    give_to_skat(g_list_nth_data(gskat.cards, order[31]));
}

/**
 * @brief Gets a provoke response from the user by opening a message box
 *
 * When the user has to 'hear' he gets the possible answers 'Yes'
 * and 'No' otherwise the next provoke value and 'Pass'.
 *
 * @param app     main application objects
 * @param value   provoking value
 * @param msg     string containing the title of the message box
 * @param hoeren  user has to 'hear'?
 *
 * @return If the user continues to provoke that value is returned.
 * Otherwise 0 is returned representing 'pass'.
 */
gint get_provoke_response(gint value, gchar *msg, gboolean hoeren)
{
    gint result;
    gchar caption[4];
    g_sprintf(caption, "%d", value);

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Reizen",
            GTK_WINDOW(gskat.allwidgets[0]),
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

/**
 * @brief Returns the next value to provoke
 *
 * @param value current provoke value
 *
 * @return the next value to provoke about
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
 * @brief Function comparing two cards used with a sort function
 *
 * @param a     first card
 * @param b     second card
 * @param data  main application objects cast to (gpointer)
 *
 * @return -1 if a > b; 1 if a < b; 0 if a == b
 */
gint compare_cards(gconstpointer a, gconstpointer b, gpointer data)
{
    card *card_a = (card *) a;
    card *card_b = (card *) b;
    struct _app *app = (struct _app *) data;

    if (!gskat.null)
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
    if (gskat.trump > 0)
    {
        if (card_a->suit == gskat.trump && card_b->suit != gskat.trump)
            return -1;
        else if (card_a->suit != gskat.trump && card_b->suit == gskat.trump)
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

/**
 * @brief Function comparing two jacks used with a sort function
 *
 * @param a first jack
 * @param b second jack
 *
 * @return -1 if a > b; 1 if a < b; 0 if a == b
 */
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

/**
 * @brief Function comparing two cards of the same suit
 * used with a sort function
 *
 * @param a first card
 * @param b second card
 *
 * @return -1 if a > b; 1 if a < b; 0 if a == b
 */
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

/**
 * @brief Returns a list of jacks of a given list of cards
 *
 * @param list list of cards to search jacks in
 *
 * @return Returns a new GList* containing all jacks in 'list'.
 * If no jacks could be found NULL is returned.
 */
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

/**
 * @brief Returns a list containing only cards of a specific suit
 *
 * @param app   main application objects
 * @param list  list of cards to search in
 * @param suit  suit that is searched
 *
 * @return Returns a new GList* containing all card of the given
 * suit. If no cards could be found NULL is returned.
 */
GList *get_suit_list(GList *list, gint suit)
{
    GList *ptr = NULL, *family = NULL;
    card *card = NULL;

    for (ptr = g_list_first(list); ptr; ptr = ptr->next)
    {
        card = ptr->data;

        if (!gskat.null)
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

/**
 * @brief Checks if the given card is trump
 *
 * @param app   main application objects
 * @param card  card to be checked
 *
 * @return TRUE if the card is trump, otherwise FALSE
 */
gboolean is_trump(card *card)
{
    if (gskat.null)
        return FALSE;
    if (!gskat.trump)
    {
        if (card->rank == BUBE)
            return TRUE;
    }
    else
    {
        if (card->suit == gskat.trump || card->rank == BUBE)
            return TRUE;
    }
    return FALSE;
}

/**
 * @brief Returns a list of trump cards from the given card list
 *
 * @param app   main application objects
 * @param list  list of cards to search in
 *
 * @return Returns a new GList* of all trump cards. If no trump
 * cards could be found return NULL.
 */
GList *get_trump_list(GList *list)
{
    GList *ptr = NULL, *family = NULL, *jacks = NULL;
    card *card = NULL;

    if (gskat.null)
        return NULL;

    jacks = get_jack_list(list);

    if (!gskat.trump)
        return jacks;

    family = get_suit_list(list, gskat.trump);

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

/**
 * @brief Return a list of cards that are possible to play
 * based on the cards currently on the table
 *
 * @param app   main applications objects
 * @param list  list of cards to search in for possible cards
 *
 * @return Returns a new GList* with all cards that are possible
 * or allowed to play.
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
 * @brief Returns a rating of a given card deck
 *
 * This function does a simply calculation of the rating of a
 * player's card deck. Currently the longest suit, the number of
 * jacks and suits you can trump are respected.
 *
 * @param app     main application objects
 * @param player  player to calculate the cards for
 * @param list    list of the player's cards
 *
 * @return card deck rating
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
 * @brief find the best suit in a list of cards
 *
 * @param list list of cards to search in
 *
 * @return index of the selected (best) suit
 */
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

/**
 * @brief Calculate the 'spitzen' for a given list of cards and suit
 *
 * @param app   main application objects
 * @param list  list of cards to parse
 * @param suit  suit to search for
 *
 * @return 'spitzen' value
 */
gint get_spitzen(GList *list, gint suit)
{
    gint i = 0, max = 0, back = 0;
    GList *ptr = NULL, *cards = NULL, *pcards = NULL;
    gboolean mit = FALSE;
    card *card = NULL, *cmp = NULL;

    /* backup trump */
    back = gskat.trump;
    gskat.trump = suit;

    cards = get_trump_list(gskat.cards);
    pcards = get_trump_list(list);

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
    gskat.trump = back;

    return (max + 1);
}

/**
 * @brief Returns the maximum value to provoke
 *
 * @param app   main application objects
 * @param list  list of cards to analyze
 *
 * @return maximum value to provoke
 */
gint get_max_reizwert(GList *list)
{
    gint suit = 0;
    gint max = 0;

    suit = get_best_suit(list);

    max = get_spitzen(list, suit);

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
 * @brief Execute the 'hearing' process for the given player
 *
 * @param app     main application objects
 * @param player  hearing player
 * @param value   provoke value that is asked
 * @param sager   saying player's index
 *
 * @return provoked value or 0 if passed
 */
gint do_hoeren(player *player, gint value, gint sager)
{
    gint max = 0;
    gint response = 0;
    gchar *msg;

    if (player->human)
    {
        msg = (gchar *) g_malloc(sizeof(gchar) *
                (20+strlen(gskat.players[sager]->name)));
        g_sprintf(msg, "%s sagt %d:", gskat.players[sager]->name, value);

        response = get_provoke_response(value, msg, TRUE);

        g_free(msg);
    }
    else
    {
        max = get_max_reizwert(player->cards);

        if (rate_cards(player, player->cards) >= 7 && value <= max)
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

/**
 * @brief Execute the 'saying' process for the given player
 *
 * @param app     main application objects
 * @param player  saying player
 * @param hoerer  hearing player's index
 * @param value   last provoked value
 *
 * @return provoked value
 */
gint do_sagen(player *player, gint hoerer, gint value)
{
    gint response = 0;
    gint gereizt = 0;
    gint id = 0;
    gint max = 0;
    gchar *msg;

    DPRINT(("Sager: %s; Hoerer: %s\n", player->name,
                gskat.players[hoerer]->name));

    /* pass immediately? */
    if (value != player->gereizt)
    {
        if (player->human)
        {
            msg = (gchar *) g_malloc(sizeof(gchar) *
                    (20+strlen(gskat.players[hoerer]->name)));
            g_sprintf(msg, "Hoerer: %s. Sagen?", gskat.players[hoerer]->name);

            gereizt = get_provoke_response(value, msg, FALSE);

            g_free(msg);
        }
        else
        {
            max = get_max_reizwert(player->cards);

            if (rate_cards(player, player->cards) >= 7 && value <= max)
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

        response = do_hoeren(gskat.players[hoerer], value, player->id);
        DPRINT(("%s sagt %s\n", gskat.players[hoerer]->name,
                (response) ? "JA" : "NEIN"));

        if (response)
        {
            id = do_sagen(player, hoerer, next_reizwert(value));
        }
        else
            return player->id;
    }

    if (player->gereizt == gskat.players[hoerer]->gereizt)
        return hoerer;
    return id;
}

/**
 * @brief Initialize provoking process
 *
 * @param app main application objects
 */
void start_provoke(app *app)
{
    gchar msg[4];
    gint hoerer = gskat.forehand;
    gint sager = (hoerer + 1) % 3;
    gint i = 18;

    DPRINT(("Start of provoking\n"));

    /* disable button */
    gtk_widget_set_sensitive(gskat.allwidgets[1], FALSE);

    /* reset all player values */
    for (i=0; i<3; ++i)
    {
        gskat.players[i]->gereizt = 0;

        DPRINT(("MaxReizwert of %s: %d\n", gskat.players[i]->name,
                    get_max_reizwert(gskat.players[i]->cards)));
        DPRINT(("CardRating of %s: %d\n", gskat.players[i]->name,
                    rate_cards(gskat.players[i], gskat.players[i]->cards)));
    }

    /* sagen */
    sager = do_sagen(gskat.players[sager], hoerer, 18);
    DPRINT(("%s won 1. reizen with %d\n", gskat.players[sager]->name,
            gskat.players[sager]->gereizt));

    sager = do_sagen(gskat.players[sager], (hoerer+2) % 3,
            (gskat.players[sager]->gereizt) ? gskat.players[sager]->gereizt : 18);

    /* first two players have passed */
    if (gskat.players[sager]->gereizt == 0)
        do_hoeren(gskat.players[sager], 18, sager);

    if (gskat.players[sager]->gereizt)
    {
        DPRINT(("%s won 2. reizen with %d\n", gskat.players[sager]->name,
                    gskat.players[sager]->gereizt));

        gskat.re = gskat.players[sager];
        gskat.re->re = TRUE;

        /* update interface */
        gtk_widget_set_sensitive(gskat.allwidgets[1], TRUE);
        gtk_button_set_label(GTK_BUTTON(gskat.allwidgets[1]), "Spiel ansagen");
        gtk_label_set_text(GTK_LABEL(gskat.allwidgets[3]), gskat.re->name);

        g_sprintf(msg, "%d", gskat.re->gereizt);
        gtk_label_set_text(GTK_LABEL(gskat.allwidgets[5]), msg);

        take_skat(app);
    }
    else
    {
        DPRINT(("All players have passed -> new round.\n"));

        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(gskat.allwidgets[0]),
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

/**
 * @brief Select cards to be put in skat
 *
 * @param app main application objects
 */
void druecke_skat(app *app)
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
 * @brief Take the two cards in the skat or play 'hand'
 *
 * @param app main application objects
 */
void take_skat(app *app)
{
    gint result;
    GList *ptr = NULL;
    card *card = NULL;

    if (gskat.re->human)
    {
        do
        {
            GtkWidget *dialog = gtk_dialog_new_with_buttons("Hand spielen?",
                    GTK_WINDOW(gskat.allwidgets[0]),
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
        druecke_skat(app);
        spiel_ansagen(app);

        gskat.state = PLAYING;
    }

    /* update screen */
    draw_area(app);
}

/**
 * @brief Decide and say what play should be played this round
 *
 * @param app main application objects
 */
void spiel_ansagen(app *app)
{
    gint result = 0;
    gchar gamename[10];
    GList *list = NULL;
    card *card = NULL;

    DPRINT(("Spiel ansagen.\n"));

    /* select game to play */
    if (gskat.re->human)
    {
        do
        {
            GtkWidget *dialog = gtk_dialog_new_with_buttons("Spiel ansagen",
                    GTK_WINDOW(gskat.allwidgets[0]),
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
    }

    /* print game to label */
    switch (gskat.trump)
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

    gtk_widget_set_sensitive(gskat.allwidgets[1], FALSE);
    gtk_label_set_text(GTK_LABEL(gskat.allwidgets[4]), gamename);

    /* hide skat & redraw screen */
    for (list = g_list_first(gskat.skat); list; list = list->next)
    {
        card = list->data;
        card->draw = FALSE;
    }

    /* reorder re player's cards */
    gskat.re->cards = g_list_sort_with_data(gskat.re->cards, compare_cards, &gskat);

    /* reorder player's cards if necessary */
    if (gskat.re != gskat.players[0])
        gskat.players[0]->cards = g_list_sort_with_data(gskat.players[0]->cards,
                compare_cards, &gskat);

    calc_card_positions();
    draw_area();

    play_stich();
}

/**
 * @brief Throw given card on the table
 *
 * @param app   main application objects
 * @param card  card to throw on the table
 */
void throw_card(card *card)
{
    player *player = gskat.players[card->owner];

    card->draw = TRUE;
    card->draw_face = TRUE;

#ifdef DEBUG
    g_print("%s played: ", player->name);
    print_card(card);
    g_print("\n");
#endif

    gskat.table = g_list_append(gskat.table, card);
    gskat.played = g_list_append(gskat.played, card);
    player->cards = g_list_remove(player->cards, card);

    calc_card_positions();
    draw_area();
}

/**
 * @brief Trigger AI to play a card
 *
 * @param app     main application objects
 * @param player  pointer to AI player
 */
void ai_play_card(player *player)
{
    GList *ptr = NULL;
    card *card = NULL;

    ptr = get_possible_cards(player->cards);

    /* TODO: implement AI logic here */
    card = ai_select_card(player, ptr);

    g_list_free(ptr);

    throw_card(card);
}

/**
 * @brief Calculate the winner and the points of the last stich
 *
 * @param app main application objects
 */
void calculate_stich()
{
    gchar msg[6];
    gint i, winner;
    gint points = 0;
    GList *ptr = NULL;
    card *crd = NULL;

    /* calculate winner of stich */
    if (is_greater(g_list_nth_data(gskat.table, 1),
                g_list_nth_data(gskat.table, 0), gskat.trump, gskat.null))
    {
        if (is_greater(g_list_nth_data(gskat.table, 2),
                    g_list_nth_data(gskat.table, 1), gskat.trump, gskat.null))
        {
            crd = g_list_nth_data(gskat.table, 2);
            winner = crd->owner;
        }
        else
        {
            crd = g_list_nth_data(gskat.table, 1);
            winner = crd->owner;
        }
    }
    else
    {
        if (is_greater(g_list_nth_data(gskat.table, 2),
                    g_list_nth_data(gskat.table, 0), gskat.trump, gskat.null))
        {
            crd = g_list_nth_data(gskat.table, 2);
            winner = crd->owner;
        }
        else
        {
            crd = g_list_nth_data(gskat.table, 0);
            winner = crd->owner;
        }
    }

    /* calculate points of stich */
    for (ptr = g_list_first(gskat.table); ptr; ptr = ptr->next)
    {
        crd = ptr->data;
        points += crd->points;

        crd->draw = FALSE;
    }

    gskat.players[winner]->points += points;
    gskat.cplayer = winner;

    DPRINT(("%s won the stich (%d).\n", gskat.players[winner]->name, points));

    /* add played cards to 'stiche' array */
    gskat.stiche[gskat.stich-1] = (card **) g_malloc(sizeof(card *) * 3);
    for (i=0; i<3; ++i)
        gskat.stiche[gskat.stich-1][i] = g_list_nth_data(gskat.table, i);

    /* remove cards from table */
    g_list_free(gskat.table);
    gskat.table = NULL;

    ++gskat.stich;

    /* update interface */
    if (gskat.stich <= 10)
    {
        g_sprintf(msg, "%d", gskat.stich);
        gtk_label_set_text(GTK_LABEL(gskat.allwidgets[2]), msg);
    }

    calc_card_positions();
    draw_area();
}

/**
 * @brief Finalize the last round by refreshing all settings
 * and player's points
 *
 * @param app main application objects
 */
void end_round()
{
    gint rank, game;
    gchar msg[200];
    GList *list = NULL, *ptr = NULL;
    card *card = NULL;
    player *player = gskat.re;

    gskat.round += 1;

    if (!gskat.null)
    {
        switch (gskat.trump)
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
        for (ptr = g_list_first(gskat.cards); ptr; ptr = ptr->next)
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
        for (ptr = g_list_first(gskat.skat); ptr; ptr = ptr->next)
        {
            card = ptr->data;

            player->points += card->points;
        }

        game = get_spitzen(list, gskat.trump);

        /* hand game */
        if (gskat.hand)
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

        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(gskat.allwidgets[0]),
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
    g_sprintf(msg, "%d", gskat.players[0]->sum_points);
    gtk_label_set_text(GTK_LABEL(gskat.allwidgets[6]), msg);
    g_sprintf(msg, "%d", gskat.players[1]->sum_points);
    gtk_label_set_text(GTK_LABEL(gskat.allwidgets[7]), msg);
    g_sprintf(msg, "%d", gskat.players[2]->sum_points);
    gtk_label_set_text(GTK_LABEL(gskat.allwidgets[8]), msg);

    g_sprintf(msg, "Runde %d", gskat.round);
    gtk_frame_set_label(GTK_FRAME(gskat.allwidgets[9]), msg);

    reset_game();
}

/**
 * @brief Trigger the next player to play his card
 *
 * @param app main application objects
 */
void play_stich()
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
            {
                ai_play_card(gskat.players[current]);
                play_stich();
            }
        }
        else
        {
            /* wait for the user to press button */
            gskat.state = READY;
        }
    }
    else
        end_round();
}

/**
 * @brief Reset all game settings and free the allocated memory
 *
 * @param app main application objects
 */
void reset_game()
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

    /* update interface */
    gtk_label_set_text(GTK_LABEL(gskat.allwidgets[2]), "1");
    gtk_label_set_text(GTK_LABEL(gskat.allwidgets[3]), "-");
    gtk_label_set_text(GTK_LABEL(gskat.allwidgets[4]), "-");
    gtk_label_set_text(GTK_LABEL(gskat.allwidgets[5]), "-");
    gtk_widget_set_sensitive(gskat.allwidgets[1], TRUE);
    gtk_button_set_label(GTK_BUTTON(gskat.allwidgets[1]), "Neue Runde");
}

/**
 * @brief Distribute cards and start a new round
 *
 * @param app main application objects
 */
void game_start()
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
    gtk_widget_set_sensitive(gskat.allwidgets[1], TRUE);
}

/* vim:set et sw=4 sts=4 tw=80: */
