#ifndef __AI_H__
#define __AI_H__

#include "def.h"

card *ai_select_card(app *, player *, GList *);

card *ai_re_kommt_raus(app *, player *, GList *);

card *ai_kontra_kommt_raus(app *, player *, GList *);

#endif /* __AI_H__ */

/* vim:set et sw=4 ts=4 tw=80: */
