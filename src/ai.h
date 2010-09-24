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

card *ai_select_card(player *player, GList *list);

card *ai_re_kommt_raus(player *player, GList *list);

card *null_re_kommt_raus(player *player, GList *list);

card *ai_kontra_kommt_raus(player *player, GList *list);

card *null_kontra_kommt_raus(player *player, GList *list);

card *ai_re_mitte(player *player, GList *list);

card *ai_kontra_mitte(player *player, GList *list);

card *null_kontra_mitte(player *player, GList *list);

card *ai_kontra_hinten(player *player, GList *list);

card *null_kontra_hinten(player *player, GList *list);

card *ai_re_hinten(player *player, GList *list);

card *null_re_hinten(player *player, GList *list);

card *null_aufspielen(player *player, GList *list);

card *kurz_aufspielen(player *player, GList *list);

card *lang_aufspielen(player *player, GList *list);

card *ai_kontra_schmieren(player *player, GList *list);

card *niedrig_bedienen(player *player, GList *list);

card *drunter_bleiben(player *player, GList *list);

card *highest_fehl(player *player, GList *list);

card *kurz_fehl_ass(player *player, GList *list);

card *highest_on_table(void);

card *knapp_trumpfen(player *player, GList *list);

card *truempfe_ziehen(player *player, GList *list);

card *trumpf_spitzen(player *player, GList *list);

card *abwerfen(player *player, GList *list);

GList *cards_out(void);

gdouble prob_stich_geht_durch(player *player);

gint num_poss_higher_cards(player *player, card *first);

gint num_jacks_played(void);

gboolean jacks_weg(player *player);

gint num_truempfe_played(void);

gboolean highest_rem_of_suit(card *first);

gboolean truempfe_weg(player *player);

gboolean hat_gestochen(player *player, gint suit);

gboolean muss_bedienen(player *player);

gboolean kommt_drueber(player *player, GList *list);

gboolean kontra_stich_sicher(player *player);

gint len_spitzen(player *player, GList *list, gint suit);

gint punkte_auf_tisch(void);

gint num_of_trump(GList *list);

gint num_of_suit(GList *list, gint suit);

gboolean re_sitzt_hinten(void);

#endif /* __AI_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
