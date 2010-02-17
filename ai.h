#ifndef __AI_H__
#define __AI_H__

#include "def.h"

card *ai_select_card(app *, player *, GList *);

card *ai_re_kommt_raus(app *, player *, GList *);

card *ai_kontra_kommt_raus(app *, player *, GList *);

card *ai_re_mitte(app *, player *, GList *);

card *ai_kontra_mitte(app *, player *, GList *);

card *ai_kontra_hinten(app *, player *, GList *);

card *ai_kontra_schmieren(app *, player *, GList *);

card *ai_re_hinten(app *, player *, GList *);

card *highest_on_table(app *);

card *knapp_trumpfen(app *, player *, GList *);

card *abwerfen(app *, player *, GList *);

GList *cards_out(app *);

gint num_poss_higher_cards(app *, player *, card *);

gboolean truempfe_draussen(app *, player *);

gboolean hat_gestochen(app *, player *, gint);

gboolean muss_bedienen(app *, player *);

gboolean kontra_stich_sicher(app *);

gint num_of_trump(app *, GList *);

gint num_of_suit(app *, GList *, gint);

#endif /* __AI_H__ */

/* vim:set et sw=4 ts=4 tw=80: */
