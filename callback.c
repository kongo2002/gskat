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

gboolean quit(GtkWidget *window, gpointer data)
{
    gtk_main_quit();

    return TRUE;
}

gboolean realization(GtkWidget *area, gpointer data)
{
    /* allocate memory for application lists */
    alloc_app();

    if (load_cards(DATA_DIR))
        game_start();

    return FALSE;
}

gboolean close_config(GtkButton *button, gpointer data)
{
    GtkWidget *window = (GtkWidget *) data;

    gtk_widget_destroy(window);

    return TRUE;
}

gboolean save_config(GtkButton *button, gpointer data)
{
    GtkWidget *window = (GtkWidget *) data;

    /* TODO: before saving we need to set the new values first */
    write_config();

    gtk_widget_destroy(window);

    return TRUE;
}

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
        /* TODO: add confirmation dialog here */
        reset_game();

        gskat.state = ENDGAME;

        next_round(button, data);
    }
}

gboolean configure(GtkWidget *area, GdkEventExpose *event, gpointer data)
{
    calc_card_positions();

    return TRUE;
}

gboolean button_press(GtkWidget *area, GdkEventButton *event, gpointer data)
{
    gboolean found = FALSE;

    if (event->button == 1)
    {
        if (gskat.state == TAKESKAT && gskat.re == gskat.players[0]
                && !gskat.hand)
            found = click_skat(event);
        else if (gskat.state == PLAYING)
            found = play_card(event);
        else if (gskat.state == READY)
        {
            calculate_stich();
            gskat.state = PLAYING;
            play_stich();
            return TRUE;
        }
    }

    return found;
}

void refresh(GtkWidget *area, GdkEventExpose *event, gpointer data)
{
    if (gskat.area && gskat.area->window)
        draw_area();
}

/* vim:set et sw=4 sts=4 tw=80: */
