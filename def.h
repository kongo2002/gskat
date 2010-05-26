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
#define DPRINT(x) /* do nothing */
#endif

static const gint SUITS[] = { KREUZ, PIK, HERZ, KARO };

/**
 * @brief Enumeration of possible card positions/orientations
 */
enum cposition
{
    CP_DIAGONAL,   /**< diagonal */
    CP_HORIZONTAL, /**< horizontal */
    CP_VERTICAL    /**< vertical */
};

/**
 * @brief Structure to represent the size and position of a card
 */
typedef struct _dimension
{
    gint x; /**< x coordinate */
    gint y; /**< y coordinate */
    gint w; /**< width */
    gint h; /**< height */
} dimension;

/**
 * @brief Enumeration of card status values
 */
enum cstatus
{
    CS_NOTAVAILABLE, /**< card is not available */
    CS_AVAILABLE,    /**< card is available */
    CS_MOVING,       /**< card is moving */
    CS_DISTRIBUTED   /**< card is distributed */
};

/**
 * @brief Enumeration of game states
 */
enum gstate
{
    LOADING,    /**< loading game parts */
    WAITING,    /**< waiting for user action */
    GIVE_CARDS, /**< distributing cards to players/skat */
    PROVOKE,    /**< players are provoking */
    TAKESKAT,   /**< player takes cards from the skat */
    PLAYING,    /**< players are playing a game round */
    READY,      /**< game is loaded */
    ENDGAME     /**< end of the game */
};

/**
 * @brief Structure representing a card
 */
typedef struct _card
{
    cairo_surface_t *img; /**< front image */
    dimension dim;        /**< current dimension/position */
    gint suit;            /**< card suit */
    gint rank;            /**< card rank */
    gint points;          /**< card value */
    gint owner;           /**< current owner of the card */
    enum cstatus status;  /**< current card status */
    gboolean draw;        /**< draw card? */
    gboolean draw_face;   /**< draw the face of the card? */
} card;

/**
 * @brief Structure containing card movement information
 */
typedef struct _card_move
{
    card *mcard; /**< card to move */
    gint dest_x; /**< x coordinate destination */
    gint dest_y; /**< y coordinate destination */
} card_move;

/**
 * @brief Structure representing a player
 */
typedef struct _player {
    gint id;         /**< player id */
    gchar *name;     /**< player name */
    gboolean human;  /**< human player? */
    gboolean re;     /**< re player? */
    gint gereizt;    /**< value the player provoked */
    gint points;     /**< points of the current round */
    gint sum_points; /**< points of all played rounds */
    GList *cards;    /**< list of player's cards */
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
    /** Configuration */
    config *conf;
} app;

extern app gskat;

#endif /*  __DEF_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
