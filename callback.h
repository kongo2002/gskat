#ifndef __CALLBACK_H__
#define __CALLBACK_H__

gboolean quit(GtkWidget *, gpointer);

void next_round(GtkButton *, gpointer);

gboolean configure(GtkWidget *, gpointer);

gboolean button_press(GtkWidget *, GdkEventButton *, gpointer);

gboolean realization(GtkWidget *, gpointer);

gboolean refresh(GtkWidget *, gpointer);

#endif /* __CALLBACK_H__ */

/* vim:set et sw=4 ts=4 tw=80: */
