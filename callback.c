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
 *
 * @param window  window triggering the event
 * @param data    arbitrary user data
 *
 * @return TRUE to not handle the event any further, otherwise FALSE
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
 *
 * @param area  GtkDrawingArea triggering the event
 * @param data  arbitrary user data
 *
 * @return TRUE to not handle the event any further, otherwise FALSE
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
 *
 * @param button  button that was clicked
 * @param data    config dialog window widget
 *
 * @return TRUE to not handle the event any further, otherwise FALSE
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
 * including the stich_view structure
 *
 * @param button  button that was clicked
 * @param data    stich_view structure
 *
 * @return TRUE to not handle the event any further, otherwise FALSE
 */
gboolean close_show_trick(GtkButton *button, gpointer data)
{
    (void) button;
    stich_view *sv = (stich_view *) data;

    gtk_widget_destroy(sv->window);
    g_free(sv);

    return TRUE;
}

/**
 * @brief Callback function of the 'previous' button
 * of the 'show last trick' dialog window
 *
 * @param button  button that was clicked
 * @param data    stich_view structure
 *
 * @return TRUE to not handle the event any further, otherwise FALSE
 */
gboolean prev_stich_click(GtkButton *button, gpointer data)
{
    stich_view *sv = (stich_view *) data;

    /* refresh the stich pointer */
    sv->stich = gskat.stiche[--sv->cur];

    /* deactivate button if on the first played stich of the round
     * or if the maximum number of viewable tricks is reached
     * according to the 'num_show_tricks' config value */
    if (sv->cur <= 0 || (gskat.stich - sv->cur) >= gskat.conf->num_show_tricks)
        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);

    /* activate next stich button */
    gtk_widget_set_sensitive(sv->nextb, TRUE);

    /* trigger stich drawing */
    draw_tricks_area(sv->area, sv);

    return TRUE;
}

/**
 * @brief Callback function of the 'previous' button
 * of the 'show last trick' dialog window
 *
 * @param button  button that was clicked
 * @param data    stich_view structure
 *
 * @return TRUE to not handle the event any further, otherwise FALSE
 */
gboolean next_stich_click(GtkButton *button, gpointer data)
{
    stich_view *sv = (stich_view *) data;

    /* refresh the stich pointer */
    sv->stich = gskat.stiche[++sv->cur];

    /* deactivate button if on the last played stich of the round */
    if (sv->cur >= (gskat.stich - 2))
        gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE);

    /* activate previous stich button */
    gtk_widget_set_sensitive(sv->prevb, TRUE);

    /* trigger stich drawing */
    draw_tricks_area(sv->area, sv);

    return TRUE;
}

/**
 * @brief Callback function of the 'apply' button of the config
 * dialog.
 *
 * Apply the set values and free the allocated memory of the confwidgets
 * array afterwards
 *
 * @param button  button that was clicked
 * @param data    config dialog window widget
 *
 * @return TRUE to not handle the event any further, otherwise FALSE
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

    gskat.conf->show_tricks = gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(gskat.confwidgets[6]));

    gskat.conf->num_show_tricks = gtk_spin_button_get_value_as_int(
            GTK_SPIN_BUTTON(gskat.confwidgets[7]));

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
 *
 * @param button  button that was clicked
 * @param data    arbitrary user data
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
 *
 * @param area   GtkDrawingArea triggering the event
 * @param event  expose event structure
 * @param data   arbitrary user data
 *
 * @return TRUE to not handle the event any further, otherwise FALSE
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
 *
 * @param area   GtkDrawingArea triggering the event
 * @param event  mouse event structure
 * @param data   arbitrary user data
 *
 * @return TRUE to not handle the event any further, otherwise FALSE
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
        /* show last trick(s) if activated in configuration */
        if (gskat.conf->show_tricks)
            show_last_tricks();

    return found;
}

/**
 * @brief Callback function of the 'animation' checkbox
 * in the config dialog
 *
 * @param tbutton  GtkCheckButton triggering the event
 * @param data     arbitrary user data
 */
void animation_toggle(GtkToggleButton *tbutton, gpointer data)
{
    (void) data;
    gboolean active = gtk_toggle_button_get_active(tbutton);

    gtk_widget_set_sensitive(gskat.confwidgets[4], active);
}

/**
 * @brief Callback function of the 'animation' checkbox
 * in the config dialog
 *
 * @param tbutton  GtkCheckButton triggering the event
 * @param data     arbitrary user data
 */
void show_tricks_toggle(GtkToggleButton *tbutton, gpointer data)
{
    (void) data;
    gboolean active = gtk_toggle_button_get_active(tbutton);

    gtk_widget_set_sensitive(gskat.confwidgets[7], active);
}

/**
 * @brief Redraw the game area on the 'expose' signal
 *
 * @param area   GtkDrawingArea triggering the event
 * @param event  expose event structure
 * @param data   arbitrary user data
 */
void refresh(GtkWidget *area, GdkEventExpose *event, gpointer data)
{
    (void) area;
    (void) event;
    (void) data;

    if (gskat.area && gskat.area->window)
        draw_area();
}

/**
 * @brief Redraw the card drawing area in the show last tricks dialog window
 *
 * @param area   GtkDrawingArea triggering the event
 * @param event  expose event structure
 * @param data   stich_view structure
 */
void refresh_tricks(GtkWidget *area, GdkEventExpose *event, gpointer data)
{
    (void) event;

    draw_tricks_area(area, (stich_view *) data);
}

/* vim:set et sw=4 sts=4 tw=80: */
