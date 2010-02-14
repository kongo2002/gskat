#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include "def.h"

player *init_player(gint, gchar *, gboolean);

void load_icons(app *);

void alloc_app(app *);

void alloc_target(app *);

void load_config(app *);

void create_interface(app *);

void pos_player_cards(player *, gint, gint, gint);

void calc_card_positions(app *);

void load_card(GList **, const gchar *, gint, gint, GtkWidget *);

gboolean load_cards(const gchar *, app *, GtkWidget *);

void draw_cards(app *, GList *, GdkPixmap *);

void draw_area(app *);

void free_app(app *);

#endif /* __INTERFACE_H__ */

/* vim:set et sw=4 ts=4 tw=80: */
