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

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>

#define DATA_DIR "/home/kongo/c/gskat/pixmaps"

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
#define DPRINT(x) if (gskat.conf.debug) g_printf x
#endif

static const gint SUITS[] = { KREUZ, PIK, HERZ, KARO };

/**
 * @brief Enumeration of possible card positions/orientations
 */
enum cposition
{
    /** diagonal */
    CP_DIAGONAL,
    /** horizontal */
    CP_HORIZONTAL,
    /** vertical */
    CP_VERTICAL
};

/**
 * @brief Structure to represent the size and position of a card
 */
typedef struct _dimension
{
    /** x coordinate */
    gint x;
    /** y coordinate */
    gint y;
    /** width */
    gint w;
    /** height */
    gint h;
} dimension;

/**
 * @brief Enumeration of card status values
 */
enum cstatus
{
    /** card is not available */
    CS_NOTAVAILABLE,
    /** card is available */
    CS_AVAILABLE,
    /** card is moving */
    CS_MOVING,
    /** card is distributed */
    CS_DISTRIBUTED
};

/**
 * @brief Enumeration of game states
 */
enum gstate
{
    /** loading game parts */
    LOADING,
    /** waiting for user action */
    WAITING,
    /** distributing cards to players/skat */
    GIVE_CARDS,
    /** players are provoking */
    PROVOKE,
    /** player takes cards from the skat */
    TAKESKAT,
    /** players are playing a game round */
    PLAYING,
    /** game is loaded */
    READY,
    /** end of the game */
    ENDGAME
};

/**
 * @brief Enumeration of game finish types
 */
enum finish_type
{
    /** game ended normally after 10 rounds */
    FT_NORMAL,
    /** re player won the game prematurely */
    FT_WIN,
    /** re player lost the game prematurely */
    FT_LOST
};

/**
 * @brief Structure representing a card
 */
typedef struct _card
{
    /** front image */
    cairo_surface_t *img;
    /** current dimension/position */
    dimension dim;
    /** card suit */
    gint suit;
    /** card rank */
    gint rank;
    /** card value */
    gint points;
    /** current owner of the card */
    gint owner;
    /** current card status */
    enum cstatus status;
    /** draw card? */
    gboolean draw;
    /** draw the face of the card? */
    gboolean draw_face;
} card;

/**
 * @brief Structure containing card movement information
 */
typedef struct _card_move
{
    /** card to move */
    card *mcard;
    /** x coordinate destination */
    gint dest_x;
    /** y coordinate destination */
    gint dest_y;
    /** card movement step in x direction*/
    gint x_move;
    /** card movement step in y direction*/
    gint y_move;
} card_move;

/**
 * @brief Structure containing stich view information
 */
typedef struct _stich_view
{
    /** index of the currently viewed stich */
    gint cur;
    /** card array of the current stich */
    card **stich;
    /** dialog window widget */
    GtkWidget *window;
    /** GtkDrawingArea where the cards are drawn */
    GtkWidget *area;
    /** previous stich button */
    GtkWidget *prevb;
    /** next stich button */
    GtkWidget *nextb;
} stich_view;

/**
 * @brief Structure representing a player
 */
typedef struct _player {
    /** player id */
    gint id;
    /** player name */
    gchar *name;
    /** human player? */
    gboolean human;
    /** re player? */
    gboolean re;
    /** value the player provoked */
    gint gereizt;
    /** points of the current round */
    gint points;
    /** points of each played round */
    GList *round_points;
    /** points of all played rounds */
    gint sum_points;
    /** list of player's cards */
    GList *cards;
} player;

/**
 * @brief Structure to represent all configuration values
 */
typedef struct _config
{
    /** Array of player names */
    gchar **player_names;
    /** GUI mode */
    gboolean gui;
    /** Card animation */
    gboolean animation;
    /** Show played stiche? */
    gboolean show_tricks;
    /** Number of played tricks to show */
    gint num_show_tricks;
    /** Card animation duration */
    gint anim_duration;
    /** Print debug statements */
    gboolean debug;
    /** Filename of the configuration file */
    gchar *filename;
} config;

/**
 * @brief Structure containing all game objects
 */
typedef struct _app
{
    /** All cards in the game */
    GList *cards;
    /** Cards in skat */
    GList *skat;
    /** Cards on the table */
    GList *table;
    /** Played cards of current round */
    GList *played;
    /** Played stiche of current round */
    card ***stiche;
    /** List of players */
    player **players;
    /** Array of 4 cells for icons */
    GdkPixbuf **icons;
    /** Image of the back of the cards */
    cairo_surface_t *back;
    /** Background image */
    cairo_surface_t *bg;
    /** Game area, is replaced by allwidgets */
    GtkWidget *area;
    /** Array of all gtk widgets used */
    GtkWidget **allwidgets;
    /** Array of configuration widgets */
    GtkWidget **confwidgets;
    /** Current game state */
    enum gstate state;
    /** Current re player */
    player *re;
    /** Current forehand player index */
    gint forehand;
    /** Current player index */
    gint cplayer;
    /** Current trump suit */
    gint trump;
    /** Round number */
    gint round;
    /** Stich number */
    gint stich;
    /** Hand game */
    gboolean hand;
    /** Null game */
    gboolean null;
    /** Configuration structure */
    config conf;
} app;

extern app gskat;

#endif /*  __DEF_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
