#ifndef __GAME_H__
#define __GAME_H__

void give_cards(app *);

void give_to_skat(app *, card *);

gboolean play_card(app *, GdkEventButton *);

gboolean click_skat(app *, GdkEventButton *);

card *click_card(app *, GdkEventButton *, GList *);

void card_to_player(player *, card *);

gint compare_cards(gconstpointer, gconstpointer, gpointer);

gint compare_jacks(gconstpointer, gconstpointer);

gint compare_family(gconstpointer, gconstpointer);

gboolean is_trump(card *, gint);

GList *get_jack_list(GList *);

GList *get_suit_list(GList *, gint);

GList *get_trump_list(GList *, gint);

GList *get_possible_cards(app *, GList *);

gint get_best_suit(GList *);

gint rate_cards(GList *);

gint get_max_reizwert(GList *);

gint next_reizwert(gint);

gint do_hoeren(app *, player *, gint);

gint do_sagen(app *, player *, gint, gint);

void take_skat(app *);

void start_provoke(app *app);

void calculate_stich(app *app);

void druecke_skat(app *);

void throw_card(app *, card *);

void ai_play_card(app *, player *);

void spiel_ansagen(app *);

void play_stich(app *);

void game_start(app *);

#endif /* __GAME_H__ */
