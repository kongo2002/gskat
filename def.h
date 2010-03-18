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

#ifndef __DEF_H__
#define __DEF_H__

#include <gtk/gtk.h>

#define DATA_DIR "/home/kongo/cpp/c/gskat/pixmaps"

#define KARO  40
#define HERZ  60
#define PIK   80
#define KREUZ 100

#define ASS    1
#define BUBE   11
#define DAME   12
#define KOENIG 13

#ifdef DEBUG
#define DPRINT(x) g_printf x
#else
#define DPRINT(x) /* do nothing */
#endif

static const gint SUITS[] = { KREUZ, PIK, HERZ, KARO };

enum cposition
{
    CP_DIAGONAL,
    CP_HORIZONTAL,
    CP_VERTICAL
};

typedef struct _dimension
{
    gint x;
    gint y;
    gint w;
    gint h;
} dimension;

enum cstatus
{
    CS_NOTAVAILABLE,
    CS_AVAILABLE,
    CS_DISTRIBUTED
};

enum gstate
{ 
    LOADING,
    WAITING,
    GIVE_CARDS,
    PROVOKE,
    TAKESKAT,
    PLAYING,
    READY,
    ENDGAME
};

typedef struct _card
{
    cairo_surface_t *img;
    dimension dim;
    gint suit;
    gint rank;
    gint points;
    gint owner;
    enum cstatus status;
    gboolean draw;
    gboolean draw_face;
    gboolean blink;
} card;

typedef struct _player {
    gint id;
    gchar *name;
    gboolean human;
    gboolean re;
    gint gereizt;
    gint points;
    gint sum_points;
    GList *cards;
} player;

typedef struct _config 
{
    /* array of player names */
    gchar **player_names;
    /* gui mode */
    gboolean gui;
} config;

typedef struct _app
{
    /* all cards in the game */
    GList *cards;
    /* cards in skat */
    GList *skat;
    /* cards on the table */
    GList *table;
    /* played cards of current round */
    GList *played;
    /* played stiche of current round */
    card ***stiche;
    /* list of players */
    player **players;
    /* array of 4 cells for icons */
    GdkPixbuf **icons;
    /* image of the back of the cards */
    cairo_surface_t *back;
    /* background image */
    cairo_surface_t *bg;
    /* game area, is replaced by allwidgets */
    GtkWidget *area;
    /* array of all gtk widgets used */
    GtkWidget **allwidgets;
    /* current game state */
    enum gstate state;
    /* current re player */
    player *re;
    /* current forehand player index */
    gint forehand;
    /* current player index */
    gint cplayer;
    /* current trump suit */
    gint trump;
    /* round number */
    gint round;
    /* stich number */
    gint stich;
    /* hand game */
    gboolean hand;
    /* null game */
    gboolean null;
    /* configuration */
    config *conf;
} app;

#endif /*  __DEF_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
