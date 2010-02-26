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

#ifndef __AI_H__
#define __AI_H__

#include "def.h"

card *ai_select_card(app *, player *, GList *);

card *ai_re_kommt_raus(app *, player *, GList *);

card *ai_kontra_kommt_raus(app *, player *, GList *);

card *ai_re_mitte(app *, player *, GList *);

card *ai_kontra_mitte(app *, player *, GList *);

card *ai_kontra_hinten(app *, player *, GList *);

card *kurz_aufspielen(app *, player *, GList *);

card *lang_aufspielen(app *, player *, GList *);

card *ai_kontra_schmieren(app *, player *, GList *);

card *ai_re_hinten(app *, player *, GList *);

card *highest_fehl(app *, player *, GList *);

card *kurz_fehl_ass(app *, player *, GList *);

card *highest_on_table(app *);

card *knapp_trumpfen(app *, player *, GList *);

card *truempfe_ziehen(app *, player *, GList *);

card *trumpf_spitzen(app *, player *, GList *);

card *abwerfen(app *, player *, GList *);

GList *cards_out(app *);

gdouble prob_stich_geht_durch(app *, player *);

gint num_poss_higher_cards(app *, player *, card *);

gint num_jacks_played(app *);

gboolean jacks_weg(app *, player *);

gint num_truempfe_played(app *);

gboolean highest_rem_of_suit(app *, card *);

gboolean truempfe_weg(app *, player *);

gboolean hat_gestochen(app *, player *, gint);

gboolean muss_bedienen(app *, player *);

gboolean kontra_stich_sicher(app *, player *);

gint len_spitzen(app *, player *, GList *, gint);

gint punkte_auf_tisch(app *);

gint num_of_trump(app *, GList *);

gint num_of_suit(app *, GList *, gint);

#endif /* __AI_H__ */

/* vim:set et sw=4 sts=4 ts=8 tw=80: */
