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
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "config.h"

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
 * cposition:
 * @CP_DIAGONAL:   Diagonal orientation
 * @CP_HORIZONTAL: Horizontal orientation
 * @CP_VERTICAL:   Vertical orientation
 *
 * Enumeration of possible card positions/orientations
 */
enum cposition {
    CP_DIAGONAL,
    CP_HORIZONTAL,
    CP_VERTICAL
};

/**
 * dimension:
 * @x: X coordinate
 * @y: Y coordinate
 * @w: Width
 * @h: Height
 *
 * Structure to represent the size and position of a card
 */
typedef struct _dimension {
    gint x;
    gint y;
    gint w;
    gint h;
} dimension;

/**
 * cstatus:
 * @CS_NOTAVAILABLE: Card is not available
 * @CS_AVAILABLE:    Card is available
 * @CS_MOVING:       Card is moving
 * @CS_DISTRIBUTED:  Card is distributed
 *
 * Enumeration of card status values
 */
enum cstatus {
    CS_NOTAVAILABLE,
    CS_AVAILABLE,
    CS_MOVING,
    CS_DISTRIBUTED
};

/**
 * gstate:
 * @LOADING:    Loading game objects
 * @WAITING:    Waiting for user action
 * @GIVE_CARDS: Distributing cards to players and 'skat'
 * @PROVOKE:    Players are bidding for the game
 * @TAKESKAT:   Player takes cards from the skat
 * @PLAYING:    Players are playing a game round
 * @READY:      Game is loaded
 * @ENDGAME:    End of the round
 *
 * Enumeration of game states
 */
enum gstate {
    LOADING,
    WAITING,
    GIVE_CARDS,
    PROVOKE,
    TAKESKAT,
    PLAYING,
    READY,
    ENDGAME
};

/**
 * finish_type:
 * @FT_NORMAL: Game round ended after 10 played rounds
 * @FT_WIN:    Re player won the game prematurely
 * @FT_LOST:   Re player lost the game prematurely
 *
 * Enumeration of game finish types
 */
enum finish_type {
    /** game ended normally after 10 rounds */
    FT_NORMAL,
    /** re player won the game prematurely */
    FT_WIN,
    /** re player lost the game prematurely */
    FT_LOST
};

/**
 * card:
 * @img:       Front image of the card
 * @dim:       Current dimension/position of the card
 * @suit:      Card suit
 * @rank:      Card rank
 * @points:    Card value
 * @owner:     Current owner of the card
 * @status:    Current card status
 * @draw:      Draw the card?
 * @draw_face: Draw the face (front) of the card?
 *
 * Structure representing a card
 */
typedef struct _card {
    cairo_surface_t *img;
    dimension dim;
    gint suit;
    gint rank;
    gint points;
    gint owner;
    enum cstatus status;
    gboolean draw;
    gboolean draw_face;
} card;

/**
 * card_move:
 * @mcard:  #card to move
 * @dest_x: X coordinate destination
 * @dest_y: Y coordinate destination
 * @x_move: card movement step in x direction
 * @y_move: card movement step in y direction
 *
 * Structure containing card movement information
 */
typedef struct _card_move {
    card *mcard;
    gint dest_x;
    gint dest_y;
    gint x_move;
    gint y_move;
} card_move;

/**
 * stich_view:
 * @cur:    Index of the currently viewed trick
 * @stich:  #card array of the current trick
 * @window: Dialog window widget
 * @area:   #GtkDrawingArea widget where the cards are drawn on
 * @prevb:  Previous #GtkButton
 * @nextb:  Next #GtkButton
 *
 * Structure containing stich view information
 */
typedef struct _stich_view {
    gint cur;
    card **stich;
    GtkWidget *window;
    GtkWidget *area;
    GtkWidget *prevb;
    GtkWidget *nextb;
} stich_view;

/**
 * player:
 * @id:           Player id
 * @name:         Player name
 * @human:        Human player?
 * @re:           Re player?
 * @gereizt:      value the player bid
 * @points:       points of the current round
 * @round_points: points of each played round
 * @sum_points:   points of all played rounds
 * @cards:        #GList containing player's cards
 *
 * Structure representing a player
 */
typedef struct _player {
    gint id;
    gchar *name;
    gboolean human;
    gboolean re;
    gint gereizt;
    gint points;
    GList *round_points;
    gint sum_points;
    GList *cards;
} player;

/**
 * config:
 * @player_names:      #gchar array of players' names
 * @animation:         Card movement animation
 * @anim_duration:     Card animation duration
 * @reaction:          Delay opponents reaction?
 * @reaction_duration: Reaction time
 * @show_tricks:       Show played tricks?
 * @num_show_tricks:   Number of showed tricks
 * @show_poss_cards:   Indicate possible cards by changing the mouse cursor shape
 * @debug:             Print debug statements
 * @filename:          Configuration filename
 *
 * Structure representing all configuration values
 */
typedef struct _config {
    gchar **player_names;
    gboolean animation;
    gint anim_duration;
    gboolean reaction;
    gint reaction_duration;
    gboolean show_tricks;
    gint num_show_tricks;
    gboolean show_poss_cards;
    gboolean debug;
    gchar *filename;
} config;

/**
 * app:
 * @cards:         #GList of all game cards
 * @skat:          #GList of cards in the 'skat'
 * @table:         #GList of all cards on the table
 * @played:        #GList of played cards of the current round
 * @stiche:        Played tricks of the current round
 * @players:       #player array of all three players
 * @icons:         #GdkPixbuf array of the four suits
 * @back:          Image of the back of the game cards
 * @bg:            Background image
 * @pirate_cursor: Alternative cursor shape for unallowed actions
 * @hand_cursor:   Alternative cursor shape for allowed actions
 * @area:          Game drawing area
 * @widgets:       #GtkWidget array of all used gtk widgets
 * @confwidgets:   #GtkWidget array of all used configuration widgets
 * @state:         Current game state
 * @re:            Current re player
 * @forehand:      Current forehand player index
 * @cplayer:       Current player index
 * @trump:         Current trump
 * @round:         Current round
 * @stich:         Current trick
 * @hand:          Current round a hand game?
 * @null:          Current round a null game?
 * @conf:          #config structure of all configuration values
 *
 * Structure containing all game objects
 */
typedef struct _app {
    GList *cards;
    GList *skat;
    GList *table;
    GList *played;
    card ***stiche;
    player **players;
    GdkPixbuf **icons;
    cairo_surface_t *back;
    cairo_surface_t *bg;
    GdkCursor *pirate_cursor;
    GdkCursor *hand_cursor;
    GtkWidget *area;
    GtkWidget **widgets;
    GtkWidget **confwidgets;
    enum gstate state;
    player *re;
    gint forehand;
    gint cplayer;
    gint trump;
    gint round;
    gint stich;
    gboolean hand;
    gboolean null;
    config conf;
} app;

/**
 * gskat:
 *
 * Main #app game object structure
 */
extern app gskat;

#endif /*  __DEF_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
