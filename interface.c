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

#include "def.h"
#include "interface.h"
#include "utils.h"
#include "callback.h"

#define CARD_MOVE_STEP 35

player *init_player(gint id, gchar *name, gboolean human)
{
    player *new = (player *) g_malloc(sizeof(player));

    if (new)
    {
        new->id = id;
        new->name = name;
        new->human = human;
        new->re = FALSE;
        new->points = 0;
        new->sum_points = 0;
        new->gereizt = 0;
        new->cards = NULL;
    }
    else
        g_printerr("Could not create new player %s\n", name);

    return new;
}

void load_icons()
{
    gint i;
    gchar *suits[] = { "diamond", "heart", "spade", "club" };
    gchar *filename;

    filename = (gchar *) g_malloc(sizeof(gchar) * (strlen(DATA_DIR) + 20));

    if (filename)
    {
        gskat.icons = (GdkPixbuf **) g_malloc(sizeof(GdkPixbuf *) * 4);

        for (i=0; i<4; i++)
        {
            g_sprintf(filename, "%s/icon-%s.xpm", DATA_DIR, suits[i]);

            DPRINT(("Loading '%s' ... ", filename));

            if (g_file_test(filename, G_FILE_TEST_EXISTS) == TRUE)
            {
                DPRINT(("OK\n"));
                gskat.icons[i] = gdk_pixbuf_new_from_file(filename, NULL);
            }
            else
            {
                DPRINT(("FAIL\n"));
                gskat.icons[i] = NULL;
            }
        }

        g_free(filename);
    }
}

void alloc_app()
{
    gint i;

    /* initialize players */
    gskat.players = (player **) g_malloc(sizeof(player *) * 3);

    if (gskat.players)
    {
        for (i=0; i<3; ++i)
            gskat.players[i] = init_player(i, gskat.conf->player_names[i],
                    i ? FALSE : TRUE);
    }
    else
        g_printerr("Could not create players.\n");

    /* initialize played cards */
    gskat.stiche = (card ***) g_malloc(sizeof(card **) * 10);

    for (i=0; i<10; ++i)
        gskat.stiche[i] = NULL;

    /* initialize suit icons */
    load_icons();
}

static GtkWidget *create_menu()
{
    GtkWidget *menu;         /* main menu */
    GtkWidget *gmenu;        /* game submenu */
    GtkWidget *game;
    GtkWidget *new_item;
    GtkWidget *quit_item;
    GtkWidget *cmenu;        /* configuration submenu */
    GtkWidget *config;
    GtkWidget *options_item;
    GtkWidget *hmenu;        /* help submenu */
    GtkWidget *help;
    GtkWidget *about_item;

    menu = gtk_menu_bar_new();

    /* game submenu */
    new_item = gtk_menu_item_new_with_label("Neue Runde");
    g_signal_connect(G_OBJECT(new_item), "activate",
            G_CALLBACK(next_round), NULL);
    quit_item = gtk_menu_item_new_with_label("Beenden");
    g_signal_connect(G_OBJECT(quit_item), "activate", G_CALLBACK(quit), NULL);

    gmenu = gtk_menu_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(gmenu), new_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(gmenu), quit_item);

    game = gtk_menu_item_new_with_label("Spiel");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(game), gmenu);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), game);

    /* configuration submenu */
    options_item = gtk_menu_item_new_with_label("Optionen");

    cmenu = gtk_menu_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(cmenu), options_item);

    config = gtk_menu_item_new_with_label("Einstellungen");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(config), cmenu);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), config);

    /* help submenu */
    about_item = gtk_menu_item_new_with_label("Über");

    hmenu = gtk_menu_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(hmenu), about_item);

    help = gtk_menu_item_new_with_label("Hilfe");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), hmenu);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), help);

    return menu;
}

void create_interface()
{
    GtkWidget *window;
    GtkWidget *vboxmenu;
    GtkWidget *mainmenu;
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *area;
    GtkWidget *frame_game;
    GtkWidget *table_game;
    GtkWidget *lb_game_stich_left;
    GtkWidget *lb_game_re_left;
    GtkWidget *lb_game_spiel_left;
    GtkWidget *lb_game_gereizt_left;
    GtkWidget *lb_game_stich_right;
    GtkWidget *lb_game_re_right;
    GtkWidget *lb_game_spiel_right;
    GtkWidget *lb_game_gereizt_right;
    GtkWidget *frame_rank;
    GtkWidget *table_rank;
    GtkWidget *lb_rank_p1_left;
    GtkWidget *lb_rank_p2_left;
    GtkWidget *lb_rank_p3_left;
    GtkWidget *lb_rank_p1_right;
    GtkWidget *lb_rank_p2_right;
    GtkWidget *lb_rank_p3_right;
    GtkWidget *button;

    gchar *iconfile = (gchar *) g_malloc(sizeof(gchar) * strlen(DATA_DIR)+20);

    if (iconfile)
        g_sprintf(iconfile, "%s/gskat.png", DATA_DIR);

    gskat.allwidgets = (GtkWidget **) g_malloc(sizeof(GtkWidget *) * 10);

    if (gskat.allwidgets != NULL)
    {
        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), "gskat");

        if (g_file_test(iconfile, G_FILE_TEST_EXISTS))
            gtk_window_set_icon_from_file(GTK_WINDOW(window), iconfile, NULL);

        g_free(iconfile);

        vboxmenu = gtk_vbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(window), vboxmenu);

        mainmenu = create_menu();
        if (mainmenu)
            gtk_box_pack_start(GTK_BOX(vboxmenu), mainmenu, FALSE, FALSE, 0);

        hbox = gtk_hbox_new(FALSE, 0);
        /* gtk_box_pack_start(child, expand, fill, padding) */
        gtk_box_pack_start(GTK_BOX(vboxmenu), hbox, TRUE, TRUE, 0);

        area = gtk_drawing_area_new();
        gtk_box_pack_start(GTK_BOX(hbox), area, TRUE, TRUE, 2);
        gtk_widget_set_size_request(area, 450, 500);
        gtk_widget_set_double_buffered(area, TRUE);

        vbox = gtk_vbox_new(FALSE, 0);
        gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, TRUE, 2);
        gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);

        frame_game = gtk_frame_new("Runde 1");
        gtk_frame_set_label_align(GTK_FRAME(frame_game), 0.5, 0.5);
        gtk_box_pack_start(GTK_BOX(vbox), frame_game, FALSE, TRUE, 2);
        gtk_frame_set_shadow_type(GTK_FRAME(frame_game), GTK_SHADOW_ETCHED_IN);

        /* gtk_table_new(rows, columns, homogeneous) */
        table_game = gtk_table_new(4, 2, FALSE);
        gtk_container_add(GTK_CONTAINER(frame_game), table_game);
        gtk_container_set_border_width(GTK_CONTAINER(table_game), 20);
        gtk_table_set_col_spacings(GTK_TABLE(table_game), 20);
        gtk_table_set_row_spacings(GTK_TABLE(table_game), 5);

        lb_game_stich_left = gtk_label_new("Stich:");
        lb_game_re_left = gtk_label_new("Re:");
        lb_game_spiel_left = gtk_label_new("Spiel:");
        lb_game_gereizt_left = gtk_label_new("Gereizt:");

        /* gtk_misc_set_alignment(misc, xalign, yalign)
         * xalign: 0 (left) to 1 (right)
         * yalign: 0 (top) to 1 (bottom) */
        gtk_misc_set_alignment(GTK_MISC(lb_game_stich_left), 1, 0.5);
        gtk_misc_set_alignment(GTK_MISC(lb_game_re_left), 1, 0.5);
        gtk_misc_set_alignment(GTK_MISC(lb_game_spiel_left), 1, 0.5);
        gtk_misc_set_alignment(GTK_MISC(lb_game_gereizt_left), 1, 0.5);

        /* gtk_table_attach_defaults(parent, child, left, right, top, bottom) */
        gtk_table_attach_defaults(GTK_TABLE(table_game),
                lb_game_stich_left,
                0, 1, 0, 1);
        gtk_table_attach_defaults(GTK_TABLE(table_game),
                lb_game_re_left,
                0, 1, 1, 2);
        gtk_table_attach_defaults(GTK_TABLE(table_game),
                lb_game_spiel_left,
                0, 1, 2, 3);
        gtk_table_attach_defaults(GTK_TABLE(table_game),
                lb_game_gereizt_left,
                0, 1, 3, 4);

        lb_game_stich_right = gtk_label_new("1");
        lb_game_re_right = gtk_label_new("-");
        lb_game_spiel_right = gtk_label_new("-");
        lb_game_gereizt_right = gtk_label_new("-");

        gtk_misc_set_alignment(GTK_MISC(lb_game_stich_right), 1, 0.5);
        gtk_misc_set_alignment(GTK_MISC(lb_game_re_right), 1, 0.5);
        gtk_misc_set_alignment(GTK_MISC(lb_game_spiel_right), 1, 0.5);
        gtk_misc_set_alignment(GTK_MISC(lb_game_gereizt_right), 1, 0.5);

        gtk_table_attach_defaults(GTK_TABLE(table_game),
                lb_game_stich_right,
                1, 2, 0, 1);
        gtk_table_attach_defaults(GTK_TABLE(table_game),
                lb_game_re_right,
                1, 2, 1, 2);
        gtk_table_attach_defaults(GTK_TABLE(table_game),
                lb_game_spiel_right,
                1, 2, 2, 3);
        gtk_table_attach_defaults(GTK_TABLE(table_game),
                lb_game_gereizt_right,
                1, 2, 3, 4);

        frame_rank = gtk_frame_new("Spielstand");
        gtk_frame_set_label_align(GTK_FRAME(frame_rank), 0.5, 0.5);
        gtk_box_pack_start(GTK_BOX(vbox), frame_rank, TRUE, TRUE, 2);
        gtk_frame_set_shadow_type(GTK_FRAME(frame_rank), GTK_SHADOW_ETCHED_IN);

        table_rank = gtk_table_new(3, 2, TRUE);
        gtk_container_add(GTK_CONTAINER(frame_rank), table_rank);
        gtk_container_set_border_width(GTK_CONTAINER(table_rank), 20);
        gtk_table_set_col_spacings(GTK_TABLE(table_rank), 20);
        gtk_table_set_row_spacings(GTK_TABLE(table_rank), 5);

        lb_rank_p1_left = gtk_label_new(gskat.conf->player_names[0]);
        lb_rank_p2_left = gtk_label_new(gskat.conf->player_names[1]);
        lb_rank_p3_left = gtk_label_new(gskat.conf->player_names[2]);

        gtk_misc_set_alignment(GTK_MISC(lb_rank_p1_left), 1, 0.5);
        gtk_misc_set_alignment(GTK_MISC(lb_rank_p2_left), 1, 0.5);
        gtk_misc_set_alignment(GTK_MISC(lb_rank_p3_left), 1, 0.5);

        gtk_table_attach_defaults(GTK_TABLE(table_rank),
                lb_rank_p1_left,
                0, 1, 0, 1);
        gtk_table_attach_defaults(GTK_TABLE(table_rank),
                lb_rank_p2_left,
                0, 1, 1, 2);
        gtk_table_attach_defaults(GTK_TABLE(table_rank),
                lb_rank_p3_left,
                0, 1, 2, 3);

        lb_rank_p1_right = gtk_label_new("0");
        lb_rank_p2_right = gtk_label_new("0");
        lb_rank_p3_right = gtk_label_new("0");

        gtk_misc_set_alignment(GTK_MISC(lb_rank_p1_right), 1, 0.5);
        gtk_misc_set_alignment(GTK_MISC(lb_rank_p2_right), 1, 0.5);
        gtk_misc_set_alignment(GTK_MISC(lb_rank_p3_right), 1, 0.5);

        gtk_table_attach_defaults(GTK_TABLE(table_rank),
                lb_rank_p1_right,
                1, 2, 0, 1);
        gtk_table_attach_defaults(GTK_TABLE(table_rank),
                lb_rank_p2_right,
                1, 2, 1, 2);
        gtk_table_attach_defaults(GTK_TABLE(table_rank),
                lb_rank_p3_right,
                1, 2, 2, 3);

        button = gtk_button_new_with_label("Neue Runde");
        gtk_widget_set_sensitive(button, FALSE);
        gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, TRUE, 2);

        /* set game object pointers */
        gskat.area = area;

        gskat.allwidgets[0] = window;
        gskat.allwidgets[1] = button;
        gskat.allwidgets[2] = lb_game_stich_right;
        gskat.allwidgets[3] = lb_game_re_right;
        gskat.allwidgets[4] = lb_game_spiel_right;
        gskat.allwidgets[5] = lb_game_gereizt_right;
        gskat.allwidgets[6] = lb_rank_p1_right;
        gskat.allwidgets[7] = lb_rank_p2_right;
        gskat.allwidgets[8] = lb_rank_p3_right;
        gskat.allwidgets[9] = frame_game;

        /* attach signals */
        g_signal_connect(G_OBJECT(window), "destroy",
                G_CALLBACK(quit), NULL);
        g_signal_connect(G_OBJECT(area), "realize",
                G_CALLBACK(realization), NULL);
        g_signal_connect(G_OBJECT(area), "configure_event",
                G_CALLBACK(configure), NULL);
        g_signal_connect(G_OBJECT(area), "expose_event",
                G_CALLBACK(refresh), NULL);
        g_signal_connect(G_OBJECT(button), "clicked",
                G_CALLBACK(next_round), NULL);

        gtk_widget_add_events(area, GDK_BUTTON_PRESS_MASK);
        g_signal_connect(G_OBJECT(area), "button_press_event",
                G_CALLBACK(button_press), NULL);
    }
}

void load_card(GList **list, const gchar *file, gint rank, gint suit)
{
    card *tcard = (card *) g_malloc(sizeof(card));

    if (tcard != NULL)
    {
        tcard->rank = rank;
        tcard->suit = suit;
        tcard->owner = -1;

        tcard->points = get_card_points(rank);

        tcard->dim.x = 0;
        tcard->dim.y = 0;

        tcard->draw = FALSE;

        tcard->img = cairo_image_surface_create_from_png(file);
        tcard->dim.w = cairo_image_surface_get_width(tcard->img);
        tcard->dim.h = cairo_image_surface_get_height(tcard->img);

        *list = g_list_prepend(*list, (gpointer) tcard);
    }
}

gboolean load_cards(const gchar *path)
{
    GList **list = &(gskat.cards);

    gint suits[] = { 40, 60, 80, 100 };
    gint ranks[] = { 1, 7, 8, 9, 10, 11, 12, 13 };

    gint i, j, id, max = strlen(path)+30;
    gboolean error = FALSE;

    gchar *cname = (gchar *) g_malloc(sizeof(gchar) * max);

    /* create all 32 cards */
    if (cname != NULL)
    {
        for (i=0; i<4; ++i)
        {
            for (j=0; j<8; ++j)
            {
                id = suits[i] + ranks[j];
                g_sprintf(cname, "%s/%d.png", path, id);

                DPRINT(("Loading '%s' ... ", cname));

                if (g_file_test(cname, G_FILE_TEST_EXISTS))
                {
                    load_card(list, cname, ranks[j], suits[i]);
                    DPRINT(("OK\n"));
                }
                else
                {
                    error = TRUE;
                    DPRINT(("FAIL\n"));
                }
            }
        }
    }

    /* load back of cards */
    g_sprintf(cname, "%s/back.png", path);
    gskat.back = load_image(cname);

    /* load back of cards */
    g_sprintf(cname, "%s/bg.png", path);
    gskat.bg = load_image(cname);

    g_free(cname);

    return !error;
}

cairo_surface_t *load_image(gchar *filename)
{
    DPRINT(("Loading '%s' ... ", filename));

    if (g_file_test(filename, G_FILE_TEST_EXISTS))
    {
        DPRINT(("OK\n"));
        return cairo_image_surface_create_from_png(filename);
    }
    else
    {
        DPRINT(("FAIL\n"));
        return NULL;
    }
}

void pos_player_cards(player *player, gint x, gint y, gint step)
{
    GList *ptr = NULL;
    card *card = NULL;

    if (player->cards)
    {
        for (ptr = g_list_first(player->cards); ptr; ptr = ptr->next)
        {
            card = ptr->data;
            card->dim.y = y;
            card->dim.x = x;
            x += step;
        }
    }
}

void calc_card_positions()
{
    gint x, y, win_w, win_h, card_w, card_h, step;
    GList *ptr = NULL;
    card *card = NULL;
    player *player = NULL;

    if ((ptr = g_list_first(gskat.cards)) && gskat.players)
    {
        card = ptr->data;
        card_w = card->dim.w;
        card_h = card->dim.h;

        win_w = gskat.area->allocation.width;
        win_h = gskat.area->allocation.height;

        /* player 0 */
        if ((player = gskat.players[0]))
        {
            if (player->cards && g_list_length(player->cards) > 0)
            {
                step = (win_w - card_w*1.5) / g_list_length(player->cards);
                y = win_h - (card_h + 5);
                x = card_w / 2;

                pos_player_cards(player, x, y, step);
            }
        }

        /* player 1 */
        if ((player = gskat.players[1]))
        {
            step = 10;
            y = 5;
            x = 5;

            pos_player_cards(player, x, y, step);
        }

        /* player 2 */
        if ((player = gskat.players[2]))
        {
            step = -10;
            y = 5;
            x = win_w - (card_w + 5);

            pos_player_cards(player, x, y, step);
        }

        /* cards in skat */
        if (gskat.skat && g_list_length(gskat.skat) > 0)
        {
            y = win_h / 2 - card_h / 2;
            x = win_w / 2 - card_w;
            step = card_w + 5;

            ptr = g_list_first(gskat.skat);
            card = ptr->data;
            card->dim.x = x;
            card->dim.y = y;
            x += step;

            ptr = g_list_last(gskat.skat);
            card = ptr->data;
            card->dim.x = x;
            card->dim.y = y;
        }

        /* cards on the table */
        if (gskat.table && g_list_length(gskat.table) > 0)
        {
            for (ptr = g_list_first(gskat.table); ptr; ptr = ptr->next)
            {
                card = ptr->data;

                /* do not update card position while moving */
                if (card->status != CS_MOVING)
                    set_table_position(card, &card->dim.x, &card->dim.y);
            }
        }
    }
}

void set_table_position(card *card, gint *dest_x, gint *dest_y)
{
    gint card_w = card->dim.w;
    gint card_h = card->dim.h;
    gint win_w = gskat.area->allocation.width;
    gint win_h = gskat.area->allocation.height;

    gint y = win_h / 2 - card_h / 2;
    gint x = win_w / 2 - card_w / 2;

    if (card->owner == 0)
    {
        *dest_x = x;
        *dest_y = y + card_h / 3;
    }
    else if (card->owner == 1)
    {
        *dest_x = x - card_w / 3;
        *dest_y = y;
    }
    else if (card->owner == 2)
    {
        *dest_x = x + card_w / 3;
        *dest_y = y - card_h / 3;
    }
}

gboolean move_card(gpointer data)
{
    card_move *cm = (card_move *) data;
    card *ptr = cm->mcard;

    gint step;
    gint dx = ptr->dim.x - cm->dest_x;
    gint dy = ptr->dim.y - cm->dest_y;

    /* adjust x coordinate */
    if (abs(dx) < CARD_MOVE_STEP)
        ptr->dim.x = cm->dest_x;
    else
    {
        step = (dx > 0) ? -CARD_MOVE_STEP : CARD_MOVE_STEP;
        ptr->dim.x += step;
    }


    /* adjust y coordinate */
    if (abs(dy) < CARD_MOVE_STEP)
        ptr->dim.y = cm->dest_y;
    else
    {
        step = (dy > 0) ? -CARD_MOVE_STEP : CARD_MOVE_STEP;
        ptr->dim.y += step;
    }


    /* check for finished movement */
    if (ptr->dim.x == cm->dest_x && ptr->dim.y == cm->dest_y)
    {
        ptr->status = CS_AVAILABLE;

        g_free(cm);
        cm = NULL;

        return FALSE;
    }

    draw_area();

    return TRUE;
}

/* draw cards from last to first */
void draw_cards(GList *cards, cairo_t *target)
{
    card *card;
    GList *ptr;
    cairo_surface_t *img = NULL;

    for (ptr = g_list_first(cards); ptr; ptr = ptr->next)
    {
        card = ptr->data;

        if (card && card->draw)
        {
            if (!card->draw_face)
                img = gskat.back;
            else
                img = card->img;
        }

        if (img)
        {
            cairo_set_source_surface(target, img, card->dim.x, card->dim.y);
            cairo_paint(target);
        }
    }
}

void draw_player(player *player, cairo_t *cr)
{
    gchar *name;
    gint card_w, card_h, w, h;
    card *card = NULL;

    /* get screen dimensions */
    w = gskat.area->allocation.width;
    h = gskat.area->allocation.height;

    /* get card dimensions */
    card = g_list_nth_data(gskat.cards, 0);
    card_w = card->dim.w;
    card_h = card->dim.h;

    /* set font color and size */
    if (player->id == gskat.forehand)
        cairo_set_source_rgb(cr, 1.0, 0.1, 0.1);
    else
        cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);

    cairo_select_font_face(cr, "sans-serif",
            CAIRO_FONT_SLANT_NORMAL,
            CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 12);

    /* set name */
    name = (gchar *) g_malloc(sizeof(gchar) * strlen(player->name) + 6);

    if (player == gskat.re)
        g_sprintf(name, "%s (Re)", player->name);
    else
        g_sprintf(name, "%s", player->name);

    /* position text */
    if (player->id == 0)
        cairo_move_to(cr, w/2-strlen(name)*5, h-card_h-10);
    else if (player->id == 1)
        cairo_move_to(cr, 20, card_h+20);
    else
        cairo_move_to(cr, w-strlen(name)*10, card_h+20);

    /* draw text */
    cairo_show_text(cr, name);

    g_free(name);
}

void draw_area()
{
    gint i;
    cairo_t *cr;
    cairo_pattern_t *pat;
    player *player;

    cr = gdk_cairo_create(gskat.area->window);

    if (gskat.bg)
    {
        pat = cairo_pattern_create_for_surface(gskat.bg);
        cairo_pattern_set_extend (pat, CAIRO_EXTEND_REPEAT);

        cairo_set_source(cr, pat);

    }
    else
        cairo_set_source_rgb(cr, 0, 0, 0);

    cairo_rectangle(cr, 0, 0,
            gskat.area->allocation.width,
            gskat.area->allocation.height);
    cairo_fill(cr);

    if (gskat.skat)
        draw_cards(gskat.skat, cr);

    if (gskat.table)
        draw_cards(gskat.table, cr);

    if (gskat.players)
    {
        for (i=0; i<3; ++i)
        {
            player = gskat.players[i];
            draw_cards(player->cards, cr);
            draw_player(player, cr);
        }
    }

    if (pat)
        cairo_pattern_destroy(pat);
    cairo_destroy(cr);
}

void free_app()
{
    GList *ptr;
    gint i;
    card *card;

    /* free players */
    for (i=0; i<3; ++i)
    {
        g_list_free(gskat.players[i]->cards);
        g_free(gskat.players[i]);
        gskat.players[i] = NULL;
    }
    g_free(gskat.players);
    gskat.players = NULL;

    /* free cards */
    for (ptr = g_list_first(gskat.cards); ptr; ptr = ptr->next)
    {
        card = ptr->data;
        if (card && card->img)
            cairo_surface_destroy(card->img);
        g_free(card);
        card = NULL;
    }
    g_list_free(gskat.cards);
    gskat.cards = NULL;
    g_list_free(gskat.skat);
    gskat.skat = NULL;
    g_list_free(gskat.played);
    gskat.played = NULL;

    /* free player names */
    for (i=0; i<3; ++i)
    {
        g_free(gskat.conf->player_names[i]);
        gskat.conf->player_names[i] = NULL;
    }
    g_free(gskat.conf->player_names);
    gskat.conf->player_names = NULL;

    g_free(gskat.conf);
    gskat.conf = NULL;

    /* free played stiche */
    for (i=0; i<10; ++i)
    {
        if (gskat.stiche[i])
            g_free(gskat.stiche[i]);
        gskat.stiche[i] = NULL;
    }
    g_free(gskat.stiche);
    gskat.stiche = NULL;

    /* free icons */
    if (gskat.icons)
    {
        for (i=0; i<4; i++)
        {
            g_object_unref(gskat.icons[i]);
            gskat.icons[i] = NULL;
        }
        g_free(gskat.icons);
    }

    /* free remaining objects */
    if (gskat.back)
        cairo_surface_destroy(gskat.back);
    gskat.back = NULL;
    if (gskat.bg)
        cairo_surface_destroy(gskat.bg);
    gskat.bg = NULL;

    g_free(gskat.allwidgets);

    DPRINT(("Quit gskat\n"));
}

/* vim:set et sw=4 sts=4 tw=80: */
