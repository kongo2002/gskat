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
#include "config.h"
#include "callback.h"
#include "utils.h"
#include "game.h"
#include "interface.h"

/**
 * @brief Leave the gtk main loop and exit the program
 */
gboolean quit(GtkWidget *window, gpointer data)
{
    (void) window;
    (void) data;

    gtk_main_quit();

    return TRUE;
}

/**
 * @brief Allocate memory for game objects, load all cards
 * and start the first game round
 */
gboolean realization(GtkWidget *area, gpointer data)
{
    (void) area;
    (void) data;

    /* allocate memory for application lists */
    alloc_app();

    if (load_cards(DATA_DIR))
        game_start();

    return FALSE;
}

/**
 * @brief Callback function of the 'cancel' button of the config
 * dialog.
 *
 * Frees the allocated memory for the confwidgets array
 */
gboolean close_config(GtkButton *button, gpointer data)
{
    (void) button;
    GtkWidget *window = (GtkWidget *) data;

    g_free(gskat.confwidgets);
    gskat.confwidgets = NULL;

    gtk_widget_destroy(window);

    return TRUE;
}

/**
 * @brief Callback function of the 'show last trick' dialog window
 *
 * Frees all allocated memory belonging to the dialog window
 */
gboolean close_show_trick(GtkButton *button, gpointer data)
{
    (void) button;
    GtkWidget *window = (GtkWidget *) data;

    gtk_widget_destroy(window);

    return TRUE;
}

/**
 * @brief Callback function of the 'apply' button of the config
 * dialog.
 *
 * Apply the set values and free the allocated memory of the confwidgets
 * array afterwards
 */
gboolean save_config(GtkButton *button, gpointer data)
{
    (void) button;
    gint i;
    const gchar *cptr = NULL;
    GtkWidget *window = (GtkWidget *) data;

    /* change player names if differing */
    for (i=0; i<3; ++i)
    {
        cptr = gtk_entry_get_text(GTK_ENTRY(gskat.confwidgets[i]));

        if (strcmp(gskat.conf->player_names[i], cptr))
        {
            g_free(gskat.conf->player_names[i]);
            gskat.conf->player_names[i] = g_strdup(cptr);
        }
    }

    gskat.conf->animation = gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(gskat.confwidgets[3]));

    gskat.conf->anim_duration = gtk_spin_button_get_value_as_int(
            GTK_SPIN_BUTTON(gskat.confwidgets[4]));

    gskat.conf->debug = gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(gskat.confwidgets[5]));

    write_config();

    g_free(gskat.confwidgets);
    gskat.confwidgets = NULL;

    gtk_widget_destroy(window);

    return TRUE;
}

/**
 * @brief Start the next or first round of the game triggered
 * by a click on the 'New Round' button
 */
void next_round(GtkButton *button, gpointer data)
{
    if (gskat.state == ENDGAME)
    {
        game_start();

        gskat.state = WAITING;

        next_round(button, data);
    }
    else if (gskat.state == WAITING)
    {
        gskat.state = PROVOKE;

        start_provoke();
    }
    else if (gskat.state == TAKESKAT)
    {
        spiel_ansagen();

        gskat.state = PLAYING;
    }
    /* abort the current round via the menu bar "Game -> New Round" */
    else
    {
        if (game_abort())
        {
            reset_game();

            gskat.state = ENDGAME;

            next_round(button, data);
        }
    }
}

/**
 * @brief Recalculate the card positions on the game table
 * after the window being resized
 */
gboolean configure(GtkWidget *area, GdkEventExpose *event, gpointer data)
{
    (void) area;
    (void) event;
    (void) data;

    calc_card_positions();

    return TRUE;
}

/**
 * @brief Callback function of a mouse click event
 *
 * Check for the current game state and trigger the appropriate action
 */
gboolean button_press(GtkWidget *area, GdkEventButton *event, gpointer data)
{
    (void) area;
    (void) data;
    gboolean found = FALSE;

    /* left mouse button click */
    if (event->button == 1)
    {
        if (gskat.state == TAKESKAT && gskat.re == gskat.players[0]
                && !gskat.hand)
            found = click_skat(event);
        else if (gskat.state == PLAYING)
            found = play_card(event);
        else if (gskat.state == READY)
        {
            gskat.state = PLAYING;
            calculate_stich();

            /* only continue playing if game was not ended early */
            if (gskat.state == PLAYING)
                play_stich();
            return TRUE;
        }
    }
    /* right mouse button click */
    else if (event->button == 3)
    {
        show_last_tricks();
        return TRUE;
    }

    return found;
}

/**
 * @brief Callback function of the 'animation' checkbox
 * in the config dialog
 */
void animation_toggle(GtkToggleButton *tbutton, gpointer data)
{
    GtkWidget *label = (GtkWidget *) data;
    gboolean active = gtk_toggle_button_get_active(tbutton);

    gtk_widget_set_sensitive(gskat.confwidgets[4], active);
    gtk_widget_set_sensitive(label, active);
}

/**
 * @brief Redraw the game area on the 'expose' signal
 */
void refresh(GtkWidget *area, GdkEventExpose *event, gpointer data)
{
    (void) area;
    (void) event;
    (void) data;

    if (gskat.area && gskat.area->window)
        draw_area();
}

/* vim:set et sw=4 sts=4 tw=80: */
