#ifndef __UTILS_H__
#define __UTILS_H__

#include "def.h"

void swap(gint *, gint *);

gint get_card_points(gint);

gchar *get_card_rank(gint, gchar *);

gchar *get_card_suit(gint, gchar *);

void print_player_cards(player *);

void print_card(card *);

gboolean higher_rank(card *, card *);

gboolean is_greater(card *, card *, gint, gboolean);

#endif /* __UTILS_H__ */

/* vim:set et sw=4 ts=4 tw=80: */
