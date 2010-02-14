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

enum cposition
{
    CP_DIAGONAL,
    CP_HORIZONTAL,
    CP_VERTICAL
};

typedef struct _dim
{
    gint x;
    gint y;
    gint w;
    gint h;
} dim;

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
    dim dim;
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

typedef struct _app
{
    /* all cards in the game */
    GList *cards;
    /* cards in skat */
    GList *skat;
    /* cards on the table */
    GList *table;
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
    gint player;
    /* current trump suit */
    gint trump;
    /* round number */
    gint round;
    /* stich number */
    gint stich;
    /* null game */
    gboolean null;
    /* gui mode */
    gboolean gui;
    /* array of player names */
    gchar **player_names;
} app;

#endif /*  __DEF_H__ */

/* vim:set et sw=4 ts=4 tw=80: */
