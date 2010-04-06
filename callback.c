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
    struct _app *app = (struct _app *) data;

    /* allocate memory for app lists */
    alloc_app(app);

    if (load_cards(DATA_DIR, app))
        game_start(app);

    return FALSE;
}

void next_round(GtkButton *button, gpointer data)
{
    struct _app *app = (struct _app *) data;

    if (app->state == ENDGAME)
    {
        game_start(app);

        app->state = WAITING;

        next_round(button, data);
    }
    else if (app->state == WAITING)
    {
        app->state = PROVOKE;

        start_provoke(app);
    }
    else if (app->state == TAKESKAT)
    {
        spiel_ansagen(app);

        app->state = PLAYING;
    }
}

gboolean configure(GtkWidget *area, GdkEventExpose *event, gpointer data)
{
    struct _app *app = (struct _app *) data;

    calc_card_positions(app);

    return TRUE;
}

gboolean button_press(GtkWidget *area, GdkEventButton *event, gpointer data)
{
    gboolean found = FALSE;
    struct _app *app = (struct _app *) data;

    if (event->button == 1)
    {
        if (app->state == TAKESKAT && app->re == app->players[0] && !app->hand)
            found = click_skat(app, event);
        else if (app->state == PLAYING)
            found = play_card(app, event);
        else if (app->state == READY)
        {
            calculate_stich(app);
            app->state = PLAYING;
            play_stich(app);
            return TRUE;
        }
    }

    return found;
}

void refresh(GtkWidget *area, GdkEventExpose *event, gpointer data)
{
    struct _app *app = (struct _app *) data;

    if (app->area && app->area->window)
        draw_area(app);
}

/* vim:set et sw=4 sts=4 tw=80: */
