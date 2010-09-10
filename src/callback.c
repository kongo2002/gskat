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
#include "configuration.h"
#include "callback.h"
#include "utils.h"
#include "game.h"
#include "gamestate.h"
#include "interface.h"
#include "license.h"

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
    else
        gtk_main_quit();

    return FALSE;
}

/**
 * @brief Show the about dialog window
 *
 * @param menuitem  menu item emitting the signal
 * @param data      arbitrary user data
 */
void show_about_window(GtkMenuItem *menuitem, gpointer data)
{
    (void) menuitem;
    (void) data;

    gchar *img_file = g_build_filename(DATA_DIR, "icons",
            "gskat128x128.png", NULL);
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(img_file, NULL);

    const gchar *authors[] =
    {
        "Gregor Uhlenheuer <kongo2002@googlemail.com>",
        NULL
    };

    gtk_show_about_dialog(GTK_WINDOW(gskat.widgets[0]),
            "program_name", _("gskat"),
            "comments", _("Gtk skat game written in C"),
            "authors", authors,
            "artists", authors,
            "version", VERSION,
            "license", license_string,
            "logo", pixbuf,
            "website", "http://github.com/kongo2002/gskat",
            "copyright", "Copyright © 2010 Gregor Uhlenheuer.\n"
                "All Rights Reserved.",
            NULL);

    if (pixbuf)
        g_object_unref(pixbuf);
    g_free(img_file);
}

/**
 * @brief Wrapper function to 'close_config' because of the different
 * function arguments of the 'delete-event' signal
 *
 * @param widget  widget receiving the signal
 * @param event   widget triggering the signal
 * @param data    config dialog window widget
 *
 * @return TRUE to not handle the event any further, otherwise FALSE
 */
gboolean destroy_config(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    (void) widget;
    (void) event;

    return close_config(NULL, data);
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
 * @brief Wrapper function to 'close_show_trick' because of the different
 * function arguments of the 'delete-event' signal
 *
 * @param widget  widget receiving the signal
 * @param event   widget triggering the signal
 * @param data    stich_view structure
 *
 * @return TRUE to not handle the event any further, otherwise FALSE
 */
gboolean destroy_show_trick(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    (void) widget;
    (void) event;

    return close_show_trick(NULL, data);
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
    if (sv->cur <= 0 || (gskat.stich - sv->cur) >= gskat.conf.num_show_tricks)
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

        if (strcmp(gskat.conf.player_names[i], cptr))
        {
            g_free(gskat.conf.player_names[i]);
            gskat.conf.player_names[i] = g_strdup(cptr);

            /* refresh game area when a player name has changed */
            draw_area();
        }
    }

    gskat.conf.animation = gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(gskat.confwidgets[3]));

    gskat.conf.anim_duration = gtk_spin_button_get_value_as_int(
            GTK_SPIN_BUTTON(gskat.confwidgets[4]));

    gskat.conf.debug = gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(gskat.confwidgets[5]));

    gskat.conf.show_tricks = gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(gskat.confwidgets[6]));

    gskat.conf.num_show_tricks = gtk_spin_button_get_value_as_int(
            GTK_SPIN_BUTTON(gskat.confwidgets[7]));

    gskat.conf.show_poss_cards = gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(gskat.confwidgets[8]));

    gskat.conf.reaction = gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(gskat.confwidgets[9]));

    gskat.conf.reaction_duration = gtk_spin_button_get_value_as_int(
            GTK_SPIN_BUTTON(gskat.confwidgets[10]));

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

        start_bidding();
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
 * @brief Callback function of a mouse move event in the drawing area
 *
 * If the cursor is moving over a non-valid card the mouse cursor shape
 * is changed to a cross (or something the like).
 *
 * @param area   GtkDrawingArea triggering the event
 * @param event  event motion structure
 * @param data   arbitrary user data
 *
 * @return TRUE to not handle the event any further, otherwise FALSE
 */
gboolean mouse_move(GtkWidget *area, GdkEventMotion *event, gpointer data)
{
    (void) data;
    gint num_cards = (gskat.table) ? g_list_length(gskat.table) : 0;
    GList *poss = NULL, *ptr;
    GdkWindow *window = area->window;
    GdkCursor *cursor = gdk_window_get_cursor(window);
    card *card;

    /* return if not enabled in configuration */
    if (!gskat.conf.show_poss_cards)
        return FALSE;

    /* check if it's the player's turn */
    if (gskat.state == PLAYING && ((gskat.cplayer + num_cards) % 3 == 0))
    {
        /* get possible cards */
        poss = get_possible_cards(gskat.players[0]->cards);

        /* iterate over player's cards */
        for (ptr = g_list_last(gskat.players[0]->cards); ptr; ptr = ptr->prev)
        {
            card = ptr->data;

            if ((gint) event->x >= card->dim.x
                    && (gint) event->y >= card->dim.y
                    && (gint) event->x < card->dim.x + card->dim.w
                    && (gint) event->y < card->dim.y + card->dim.h)
            {
                if (g_list_index(poss, card) == -1)
                {
                    /* set cross cursor if not already set */
                    if (!cursor || cursor->type != GDK_PIRATE)
                        gdk_window_set_cursor(window, gskat.pirate_cursor);

                    if (poss)
                        g_list_free(poss);

                    return FALSE;
                }
                else
                {
                    /* set hand cursor if not already set */
                    if (!cursor || cursor->type != GDK_HAND1)
                        gdk_window_set_cursor(window, gskat.hand_cursor);

                    if (poss)
                        g_list_free(poss);

                    return FALSE;
                }
            }
        }
    }

    /* reset to default cursor if necessary */
    if (cursor)
        gdk_window_set_cursor(window, NULL);

    if (poss)
        g_list_free(poss);

    return FALSE;
}

/**
 * @brief Callback function of a mouse click event in the drawing area
 *
 * Check for the current game state and trigger the appropriate action
 *
 * @param area   GtkDrawingArea triggering the event
 * @param event  mouse event structure
 * @param data   arbitrary user data
 *
 * @return TRUE to not handle the event any further, otherwise FALSE
 */
gboolean mouse_click(GtkWidget *area, GdkEventButton *event, gpointer data)
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
        if (gskat.conf.show_tricks)
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
 * @brief Callback function of the 'reaction' checkbox
 * in the config dialog
 *
 * @param tbutton  GtkCheckButton triggering the event
 * @param data     arbitrary user data
 */
void reaction_toggle(GtkToggleButton *tbutton, gpointer data)
{
    (void) data;
    gboolean active = gtk_toggle_button_get_active(tbutton);

    gtk_widget_set_sensitive(gskat.confwidgets[10], active);
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

/**
 * @brief Load a saved game state
 *
 * @param menuitem  the menu item which received the signal
 * @param data      arbitrary user data
 */
void load_game_cb(GtkMenuItem *menuitem, gpointer data)
{
    gchar *filename = g_build_filename(get_config_dir(), "gamestate", NULL);

    if (read_state_from_file(filename))
    {
        gskat.state = PLAYING;

        update_interface();
        calc_card_positions();
        draw_area();

        play_stich();

        gtk_widget_set_sensitive(gskat.widgets[15], TRUE);
    }

    g_free(filename);
}

/**
 * @brief Save the current game state
 *
 * @param menuitem  the menu item which received the signal
 * @param data      arbitrary user data
 */
void save_game_cb(GtkMenuItem *menuitem, gpointer data)
{
    gchar *filename = g_build_filename(get_config_dir(), "gamestate", NULL);

    save_state_to_file(filename);

    g_free(filename);
}

/* vim:set et sw=4 sts=4 tw=80: */
