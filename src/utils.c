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
 * swap:
 * @a:  first integer
 * @b:  second integer
 *
 * Helper function to swap two integers
 */
void swap(gint *a, gint *b)
{
    gint tmp = *a;
    *a = *b;
    *b = tmp;
}

/**
 * max_str_len:
 * @first:  first string to check
 * @...:    additional strings to check
 *
 * Return maximum string length
 *
 * Returns: maximum string length of all given strings
 */
gint max_str_len(const gchar *first, ...)
{
    guint max = strlen(first);
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
 * get_config_dir:
 *
 * Determine the configuration directory
 *
 * First try to get the configuration directory according to
 * the XDG base directory specification. If that fails use the
 * home directory.
 *
 * Returns: (transfer none): configuration directory string (may not be freed)
 */
gchar *get_config_dir(void)
{
    const gchar *base_dir;
    static gchar *config_dir = NULL;

    if (!config_dir)
    {
        /* get XDG config directory */
        if ((base_dir = g_get_user_config_dir()) && *base_dir != '\0')
            config_dir = g_build_filename(base_dir, "gskat", NULL);
        else
        {
            /* get home directory */
            if (!(base_dir = g_getenv("HOME")))
                base_dir = g_get_home_dir();

            config_dir = g_build_filename(base_dir, ".gskat", NULL);
        }

        /* if the directory does not exist try to create it
         * otherwise return an empty string */
        if (!create_dir(config_dir))
            config_dir = "";
    }

    return config_dir;
}

/**
 * get_data_dir:
 *
 * Determine the data directory
 *
 * First try to get the data directory according to
 * the XDG base directory specification. If that fails use the
 * home directory.
 *
 * Returns: (transfer none): data directory string (may not be freed)
 */
gchar *get_data_dir(void)
{
    const gchar *base_dir;
    static gchar *data_dir = NULL;

    if (!data_dir)
    {
        /* get XDG data directory */
        if ((base_dir = g_get_user_data_dir()) && *base_dir != '\0')
            data_dir = g_build_filename(base_dir, "gskat", NULL);
        else
        {
            /* get home directory */
            if (!(base_dir = g_getenv("HOME")))
                base_dir = g_get_home_dir();

            data_dir = g_build_filename(base_dir, ".gskat", NULL);
        }

        /* if the directory does not exist try to create it
         * otherwise return an empty string */
        if (!create_dir(data_dir))
            data_dir = "";
    }

    return data_dir;
}

/**
 * get_cache_dir:
 *
 * Determine the cache directory
 *
 * First try to get the cache directory according to
 * the XDG base directory specification. If that fails use the
 * home directory.
 *
 * Returns: (transfer none): cache directory string (may not be freed)
 */
gchar *get_cache_dir(void)
{
    const gchar *base_dir;
    static gchar *cache_dir = NULL;

    if (!cache_dir)
    {
        /* get XDG cache directory */
        if ((base_dir = g_get_user_cache_dir()) && *base_dir != '\0')
            cache_dir = g_build_filename(base_dir, "gskat", NULL);
        else
        {
            /* get home directory */
            if (!(base_dir = g_getenv("HOME")))
                base_dir = g_get_home_dir();

            cache_dir = g_build_filename(base_dir, ".gskat", NULL);
        }

        /* if the directory does not exist try to create it
         * otherwise return an empty string */
        if (!create_dir(cache_dir))
            cache_dir = "";
    }

    return cache_dir;
}

/**
 * create_dir:
 * @dir:  directory string
 *
 * Create the given directory if necessary
 *
 * Returns: %TRUE on success, otherwise %FALSE
 */
gboolean create_dir(const gchar *dir)
{
    gboolean exists;

    exists = g_file_test(dir, G_FILE_TEST_EXISTS);

    if (!exists && g_mkdir(dir, 0755) != 0)
    {
        DPRINT((_("Unable to create directory: %s\n"), dir));
        return FALSE;
    }

    return TRUE;
}

/**
 * get_card_points:
 * @rank:  rank of the #card
 *
 * Get #card value depending on its rank
 *
 * Returns: point value of the given #card rank
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
 * suit_name:
 * @id:  suit id of the #card
 *
 * Return the card's suit name
 *
 * Returns: (transfer none): suit name
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
 * rank_name:
 * @id:  rank id of the #card
 *
 * Return the card's rank name
 *
 * Returns: (transfer none): rank name
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
 * get_card_by_id:
 * @id:  id of #card to fetch
 *
 * Convenience wrapper for the get_card_ptr function
 *
 * Returns: #card of found #card or %NULL if no matching #card was found
 */
card *get_card_by_id(gint id)
{
    gint rank = id % 20;
    gint suit = id - rank;

    return get_card_ptr(suit, rank);
}

/**
 * get_card_ptr:
 * @suit:  suit of #card to fetch
 * @rank:  rank of #card to fetch
 *
 * Get pointer to the card of given suit and rank
 *
 * Returns: #card of found #card or %NULL if no matching #card was found
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

    DPRINT((_("Could not find a matching card (Suit: %d, Rank: %d).\n"),
                suit, rank));

    return NULL;
}

/**
 * get_card_name:
 * @card:  pointer to #card to get the name of
 *
 * Return the card's name
 *
 * Returns: (transfer none): string of card name
 */
gchar *get_card_name(card *card)
{
    static gchar name[64];

    g_snprintf(name, 64, "%s %s", suit_name(card->suit), rank_name(card->rank));

    return name;
}

/**
 * print_card:
 * @card:  pointer to #card to print
 *
 * Print the card's name to stdout
 */
void print_card(card *card)
{
    g_print("%s %s", suit_name(card->suit), rank_name(card->rank));
}

/**
 * print_player_cards:
 * @player: pointer to #player to print
 *
 * Print player's cards to stdout
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
 * update_sb:
 * @fmt:  printf-like format-string
 * @...:  additional arguments
 *
 * Remove the current message from the statusbar stack and
 * push a new message onto the message stack
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
 * higher_rank:
 * @top:     #card lying on the top
 * @bottom:  #card lying on the bottom
 *
 * Compare two cards' ranks
 *
 * Returns: %TRUE if top is higher that bottom, otherwise %FALSE
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
 * is_greater:
 * @top:    #card lying on the top
 * @bottom: #card lying on the bottom
 * @trump:  trump suit index
 * @null:   %TRUE if it's a null game, otherwise %FALSE
 *
 * Check if the top #card is higher than the bottom #card
 * depending on trump and game type
 *
 * Returns: %TRUE if top is greater than bottom, otherwise %FALSE
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

/**
 * get_trick_winner:
 * @table:  #GList containing the three cards on the table
 *
 * Calculate the winner of the current trick on the table
 *
 * Returns: Index of the #player that won the trick
 */
gint get_trick_winner(GList *table)
{
    card *tmp;

    if (is_greater(g_list_nth_data(table, 1),
                g_list_nth_data(table, 0), gskat.trump, gskat.null))
    {
        if (is_greater(g_list_nth_data(table, 2),
                    g_list_nth_data(table, 1), gskat.trump, gskat.null))
        {
            tmp = g_list_nth_data(table, 2);
            return tmp->owner;
        }
        else
        {
            tmp = g_list_nth_data(table, 1);
            return tmp->owner;
        }
    }
    else
    {
        if (is_greater(g_list_nth_data(table, 2),
                    g_list_nth_data(table, 0), gskat.trump, gskat.null))
        {
            tmp = g_list_nth_data(table, 2);
            return tmp->owner;
        }
        else
        {
            tmp = g_list_nth_data(table, 0);
            return tmp->owner;
        }
    }
}

/* vim:set et sw=4 sts=4 tw=80: */
