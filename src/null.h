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

#ifndef __NULL_H__
#define __NULL_H__

#include "def.h"

card *null_select_card(player *player, GList *list, gint position);

card *null_re_kommt_raus(player *player, GList *list);

card *null_kontra_kommt_raus(player *player, GList *list);

card *null_kontra_mitte(player *player, GList *list);

card *null_kontra_hinten(player *player, GList *list);

card *null_re_hinten(player *player, GList *list);

card *null_aufspielen(player *player, GList *list);

card *niedrig_bedienen(player *player, GList *list);

card *drunter_bleiben(player *player, GList *list);

#endif /* __NULL_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
