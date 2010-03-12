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

card *ai_select_card(app *app, player *player, GList *list);

card *ai_re_kommt_raus(app *app, player *player, GList *list);

card *ai_kontra_kommt_raus(app *app, player *player, GList *list);

card *ai_re_mitte(app *app, player *player, GList *list);

card *ai_kontra_mitte(app *app, player *player, GList *list);

card *ai_kontra_hinten(app *app, player *player, GList *list);

card *kurz_aufspielen(app *app, player *player, GList *list);

card *lang_aufspielen(app *app, player *player, GList *list);

card *ai_kontra_schmieren(app *app, player *player, GList *list);

card *ai_re_hinten(app *app, player *player, GList *list);

card *highest_fehl(app *app, player *player, GList *list);

card *kurz_fehl_ass(app *app, player *player, GList *list);

card *highest_on_table(app *app);

card *knapp_trumpfen(app *app, player *player, GList *list);

card *truempfe_ziehen(app *app, player *player, GList *list);

card *trumpf_spitzen(app *app, player *player, GList *list);

card *abwerfen(app *app, player *player, GList *list);

GList *cards_out(app *app);

gdouble prob_stich_geht_durch(app *app, player *player);

gint num_poss_higher_cards(app *app, player *player, card *card);

gint num_jacks_played(app *app);

gboolean jacks_weg(app *app, player *player);

gint num_truempfe_played(app *app);

gboolean highest_rem_of_suit(app *app, card *card);

gboolean truempfe_weg(app *app, player *player);

gboolean hat_gestochen(app *app, player *player, gint suit);

gboolean muss_bedienen(app *app, player *player);

gboolean kontra_stich_sicher(app *app, player *player);

gint len_spitzen(app *app, player *player, GList *list, gint suit);

gint punkte_auf_tisch(app *app);

gint num_of_trump(app *app, GList *list);

gint num_of_suit(app *app, GList *list, gint suit);

#endif /* __AI_H__ */

/* vim:set et sw=4 sts=4 ts=8 tw=80: */
