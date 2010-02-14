#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include "def.h"
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

gint get_provoke_response(app *app, gint value)
{
    gchar caption[4];
    g_sprintf(caption, "%d", value);

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Reizen",
            GTK_WINDOW(app->allwidgets[0]),
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            caption, value,
            "Pass", 0,
            NULL);

    /*
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),
            gtk_label_new("Reizen"), FALSE, TRUE, 2);
    gtk_widget_show_all(dialog);
    */

    gint result = gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);
    return result;
}

gint next_reizwert(gint value)
{
    gint i;
    gint reizen[] = { 18,20,22,23,24,27,30,33,36,40,44,45,
                      48,50,54,55,60,63,66,70,72,77,80,84 };

    for (i=0; i<23; ++i)
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

    /* no trump given, i.e. before provoking */
    if (app->trump <= 0)
    {
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
    }

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

GList *get_suit_list(GList *list, gint suit)
{
    GList *ptr = NULL, *family = NULL;
    card *card = NULL;

    for (ptr = g_list_first(list); ptr; ptr = ptr->next)
    {
        card = ptr->data;
        if (card->suit == suit && card->rank != BUBE)
            family = g_list_prepend(family, (gpointer) card);
    }

    if (family)
        family = g_list_sort(family, compare_family);
    return family;
}

gboolean is_trump(card *card, gint trump)
{
    if (card->suit == trump || card->rank == BUBE)
        return TRUE;
    return FALSE;
}

GList *get_trump_list(GList *list, gint trump)
{
    GList *ptr = NULL, *family = NULL, *jacks = NULL;
    card *card = NULL;

    family = get_suit_list(list, trump);

    jacks = get_jack_list(list);

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

        if (is_trump(card, app->trump))
            ptr = get_trump_list(list, app->trump);
        else
            ptr = get_suit_list(list, card->suit);

        if (ptr)
            return ptr;
        else
            return g_list_copy(list);
    }
    return NULL;
}

gint rate_cards(GList *list)
{
    gint jacks = 0, rate = 0, best = 0;
    GList *ptr = NULL;
    card *card = NULL;

    best = get_best_suit(list);

    /* no cards of one suit? */
    if (!get_suit_list(list, KARO))
        rate++;
    if (!get_suit_list(list, HERZ))
        rate++;
    if (!get_suit_list(list, PIK))
        rate++;
    if (!get_suit_list(list, KREUZ))
        rate++;

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

gint get_spitzen(GList *list)
{
    gint max = 0;
    GList *ptr = NULL, *jacks = NULL;
    card *card = NULL;

    jacks = get_jack_list(list);

    if (!jacks)
        max = 4;
    else
    {
        ptr = g_list_first(jacks);
        card = ptr->data;

        /* mit ... */
        if (card->suit == KREUZ)
        {
            for (ptr = g_list_first(jacks); ptr; ptr = ptr->next)
            {
                card = ptr->data;
                if (card->suit == (KREUZ - 20*max))
                    ++max;
                else
                    break;
            }
        }
        /* ohne ... */
        else
        {
            ptr = g_list_first(jacks);
            card = ptr->data;
            max = (KREUZ - card->suit) / 20;
        }

        g_list_free(jacks);
    }

    return (max + 1);
}

gint get_max_reizwert(GList *list)
{
    gint suit = 0;
    gint max = 0;

    max = get_spitzen(list);

    suit = get_best_suit(list);

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

gint do_hoeren(app *app, player *player, gint value)
{
    gint max = 0;
    gint response = 0;

    if (player->human)
        response = get_provoke_response(app, value);
    else
    {
        max = get_max_reizwert(player->cards);

        if (rate_cards(player->cards) >= 6 && value <= max)
        {
            player->gereizt = value;
            return value;
        }
        else
            return 0;
    }
    return response;
}

gint do_sagen(app *app, player *player, gint hoerer, gint value)
{
    gint response = 0;
    gint gereizt = 0;
    gint id = 0;
    gint max = 0;

    DPRINT(("Sager: %s; Hoerer: %s\n", player->name,
                app->player_names[hoerer]));

    /* pass immediately? */
    if (value != player->gereizt)
    {
        if (player->human)
            gereizt = get_provoke_response(app, value);
        else
        {
            max = get_max_reizwert(player->cards);

            if (rate_cards(player->cards) >= 6 && value <= max)
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

        response = do_hoeren(app, app->players[hoerer], value);
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
                    get_max_reizwert(app->players[i]->cards)));
        DPRINT(("CardRating of %s: %d\n", app->player_names[i],
                    rate_cards(app->players[i]->cards)));
    }

    /* sagen */
    sager = do_sagen(app, app->players[sager], hoerer, 18);
    DPRINT(("%s won 1. reizen with %d\n", app->player_names[sager],
            app->players[sager]->gereizt));

    sager = do_sagen(app, app->players[sager], (hoerer+2) % 3,
            (app->players[sager]->gereizt) ? app->players[sager]->gereizt : 18);

    /* first two players have passed */
    if (app->players[sager]->gereizt == 0)
        do_hoeren(app, app->players[sager], 18);

    /* TODO: Ramsch */
    if (app->players[sager]->gereizt)
    {
        DPRINT(("%s won 2. reizen with %d\n", app->player_names[sager],
                    app->players[sager]->gereizt));

        app->re = app->players[sager];

        /* update interface */
        gtk_widget_set_sensitive(app->allwidgets[1], TRUE);
        gtk_button_set_label(GTK_BUTTON(app->allwidgets[1]), "Spiel ansagen");
        gtk_label_set_text(GTK_LABEL(app->allwidgets[3]), app->re->name);

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
            ptr = get_suit_list(player->cards, suits[i]);

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
                ptr = get_suit_list(player->cards, suits[i]);
    
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
    GList *ptr = NULL;
    card *card = NULL;

    app->state = TAKESKAT;

    for (ptr = g_list_first(app->skat); ptr; ptr = ptr->next)
    {
        /* update card owner */
        card = ptr->data;
        card->owner = app->re->id;

        /* show cards in skat */
        if (app->re->human)
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
        case 0:
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
        case 200:
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
    player->cards = g_list_remove(player->cards, card);

    calc_card_positions(app);
    draw_area(app);
}

void ai_play_card(app *app, player *player)
{
    gint sel;
    GList *ptr = NULL;
    card *card = NULL;

    ptr = get_possible_cards(app, player->cards);

    /* TODO: implement AI logic here
     * currently random card selection only */
    sel = rand() % g_list_length(ptr);
    card = g_list_nth_data(ptr, sel);

    g_list_free(ptr);

    throw_card(app, card);
}

void calculate_stich(app *app)
{
    gchar msg[6];
    gint points = 0;
    gint winner;
    GList *ptr = NULL;
    card *card = NULL;

    /* calculate winner of stich */
    if (is_greater(g_list_nth_data(app->table, 1),
                g_list_nth_data(app->table, 0), app->trump, app->null))
    {
        if (is_greater(g_list_nth_data(app->table, 2),
                    g_list_nth_data(app->table, 1), app->trump, app->null))
        {
            card = g_list_nth_data(app->table, 2);
            winner = card->owner;
        }
        else
        {
            card = g_list_nth_data(app->table, 1);
            winner = card->owner;
        }
    }
    else
    {
        if (is_greater(g_list_nth_data(app->table, 2),
                    g_list_nth_data(app->table, 0), app->trump, app->null))
        {
            card = g_list_nth_data(app->table, 2);
            winner = card->owner;
        }
        else
        {
            card = g_list_nth_data(app->table, 0);
            winner = card->owner;
        }
    }

    /* calculate points of stich */
    for (ptr = g_list_first(app->table); ptr; ptr = ptr->next)
    {
        card = ptr->data;
        points += card->points;

        card->draw = FALSE;
    }

    app->players[winner]->points += points;
    app->player = winner;

    DPRINT(("%s won the stich (%d).\n", app->player_names[winner], points));

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
            case 20:
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

        game = get_spitzen(list);

        g_list_free(list);

        /* player has won */
        if (player->points > 60)
        {
            /* player won 'schwarz' */
            if (player->points == 120)
                game += 2;
            /* player won 'schneider' */
            else if (player->points > 90)
                game += 1;

            game *= rank;

            g_sprintf(msg, "%s won the game with %d to %d points\n\t+%d",
                    player->name,
                    player->points,
                    (120 - player->points),
                    game);
        }
        /* player has lost */
        else
        {
            /* player lost 'schwarz' */
            if (player->points == 0)
                game += 2;
            /* player lost 'schneider' */
            else if (player->points < 30)
                game += 1;

            game *= rank * -2;

            g_sprintf(msg, "%s lost the game with %d to %d points\n\t%d",
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
                msg);

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
    gtk_label_set_text(GTK_LABEL(app->allwidgets[5]), msg);
    g_sprintf(msg, "%d", app->players[1]->sum_points);
    gtk_label_set_text(GTK_LABEL(app->allwidgets[6]), msg);
    g_sprintf(msg, "%d", app->players[2]->sum_points);
    gtk_label_set_text(GTK_LABEL(app->allwidgets[7]), msg);

    g_sprintf(msg, "Runde %d", app->round);
    gtk_frame_set_label(GTK_FRAME(app->allwidgets[8]), msg);

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

    /* remove cards from players if necessary */
    if (app->players)
    {
        for (i=0; i<3; ++i)
        {
            if (app->players[i]->cards)
                g_list_free(app->players[i]->cards);
            app->players[i]->cards = NULL;
            app->players[i]->gereizt = 0;
            app->players[i]->points = 0;
        }
    }

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
