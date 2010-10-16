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
#include "draw.h"
#include "utils.h"

/**
 * pos_player_cards:
 * @player:  #player to position his cards for
 * @x:       x coordinate of the starting position
 * @y:       y coordinate of the starting position
 * @step:    offset between the cards in x direction
 *
 * Position the player's cards on the game table
 */
void pos_player_cards(player *player, gint x, gint y, gint step)
{
    GList *ptr = NULL;
    card *card = NULL;

    g_return_if_fail(player->cards);

    for (ptr = g_list_first(player->cards); ptr; ptr = ptr->next)
    {
        card = ptr->data;
        card->dim.y = y;
        card->dim.x = x;
        x += step;
    }
}

/**
 * calc_card_positions:
 *
 * Calculate the card positions of all three players
 * based on the game window's dimension
 */
void calc_card_positions(void)
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
        if ((player = gskat.players[1]) && player->cards)
        {
            step = 10;
            y = 5;
            x = 5;

            pos_player_cards(player, x, y, step);
        }

        /* player 2 */
        if ((player = gskat.players[2]) && player->cards)
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

/**
 * set_table_position:
 * @card:    #card to set the table position for
 * @dest_x:  x coordinate of the destination
 * @dest_y:  y coordinate of the destination
 *
 * Set the position of the given @card on the game table
 */
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

/**
 * set_card_move_step:
 * @cm:  #card_move structure to fill
 *
 * Calculate the card movement step depending on the configuration value
 * 'anim_duration', the drawing timeout interval of 25 ms and the card
 * distance to move in x and y direction.
 */
void set_card_move_step(card_move *cm)
{
    card *ptr = cm->mcard;

    gint dx = ABS(ptr->dim.x - cm->dest_x);
    gint dy = ABS(ptr->dim.y - cm->dest_y);

    cm->x_move = (gdouble) dx / 25;
    cm->y_move = (gdouble) dy / 25;

    if (!cm->x_move)
        cm->x_move = 1;

    if (!cm->y_move)
        cm->y_move = 1;
}

/**
 * move_card:
 * @data:  #card_move structure that contains the card movement information
 *
 * Move the given card towards its destination by the given
 * movement step
 *
 * The card movement structure contains the information about what card to move,
 * the movement destination and the movement step.
 *
 * Returns: %FALSE if the card movement is finished, otherwise %TRUE
 */
gboolean move_card(gpointer data)
{
    card_move *cm = (card_move *) data;
    card *ptr = cm->mcard;

    gint step;
    gint x_move = cm->x_move;
    gint y_move = cm->y_move;
    gint dx = ptr->dim.x - cm->dest_x;
    gint dy = ptr->dim.y - cm->dest_y;

    /* adjust x coordinate */
    if (ABS(dx) < x_move)
        ptr->dim.x = cm->dest_x;
    else
    {
        step = (dx > 0) ? -x_move : x_move;
        ptr->dim.x += step;
    }

    /* adjust y coordinate */
    if (ABS(dy) < y_move)
        ptr->dim.y = cm->dest_y;
    else
    {
        step = (dy > 0) ? -y_move : y_move;
        ptr->dim.y += step;
    }

    /* check for finished movement */
    if (ptr->dim.x == cm->dest_x && ptr->dim.y == cm->dest_y)
    {
        ptr->status = CS_AVAILABLE;
        draw_area();

        g_free(cm);
        return FALSE;
    }

    draw_area();

    return TRUE;
}

/**
 * draw_cards:
 * @cards:   #GList of cards to draw
 * @target:  #cairo_t drawing object
 *
 * Draw the cards from the last to the first
 */
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

/**
 * draw_player:
 * @player:  #player to draw the name of
 * @cr:      #cairo_t drawing object
 *
 * Draw @player's name on the game surface
 *
 * The #player in the forehand position is drawn in red and
 * the others in black.
 */
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
        g_sprintf(name, _("%s (Re)"), player->name);
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

/**
 * draw_table:
 * @area:  #GtkDrawingArea widget the background is drawn on
 * @cr:    #cairo_t drawing object
 *
 * Draw the table background
 */
void draw_table(GtkWidget *area, cairo_t *cr)
{
    cairo_pattern_t *pat = NULL;

    if (gskat.bg)
    {
        pat = cairo_pattern_create_for_surface(gskat.bg);
        cairo_pattern_set_extend (pat, CAIRO_EXTEND_REPEAT);

        cairo_set_source(cr, pat);

    }
    else
        cairo_set_source_rgb(cr, 0, 0, 0);

    cairo_rectangle(cr, 0, 0,
            area->allocation.width,
            area->allocation.height);
    cairo_fill(cr);

    if (pat)
        cairo_pattern_destroy(pat);
}

/**
 * draw_area:
 *
 * Draw the game area with its players and their cards
 */
void draw_area(void)
{
    gint i;
    cairo_t *cr;
    player *player;

    GdkRectangle rect =
    {
        0, 0,
        gskat.area->allocation.width,
        gskat.area->allocation.height
    };

    gdk_window_begin_paint_rect(gskat.area->window, &rect);

    cr = gdk_cairo_create(gskat.area->window);

    draw_table(gskat.area, cr);

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

    cairo_destroy(cr);

    gdk_window_end_paint(gskat.area->window);
}

/**
 * draw_tricks_area:
 * @area: #GtkDrawingArea widget the cards are drawn on
 * @sv:   trick (three cards) to draw
 *
 * Draw the tricks in the show last tricks dialog window
 */
void draw_tricks_area(GtkWidget *area, stich_view *sv)
{
    gint i, x, y, winner;
    gchar *caption = NULL;
    cairo_t *cr;
    card **stich = sv->stich;

    GdkRectangle rect =
    {
        0, 0,
        area->allocation.width,
        area->allocation.height
    };

    /* begin drawing process
     * used for double buffering */
    gdk_window_begin_paint_rect(area->window, &rect);

    cr = gdk_cairo_create(area->window);

    /* draw table background */
    draw_table(area, cr);

    /* draw current stich */
    caption = g_strdup_printf(_("Trick %d"), sv->cur + 1);

    cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
    cairo_select_font_face(cr, "sans-serif",
            CAIRO_FONT_SLANT_NORMAL,
            CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 12);
    cairo_move_to(cr, area->allocation.width / 2 - 25, 25);
    cairo_show_text(cr, caption);

    /* draw cards of the given stich */
    x = 5;
    y = 40;
    winner = get_trick_winner(stich);

    for (i=0; i<3; ++i)
    {
        if (stich[i])
        {
            /* draw card image */
            cairo_set_source_surface(cr, stich[i]->img, x, y);
            cairo_paint(cr);

            /* darken the non-winning cards */
            if (stich[i]->owner != winner)
            {
                cairo_set_source_rgba(cr, 0.1, 0.1, 0.1, 0.2);
                cairo_rectangle(cr, x, y, stich[i]->dim.w, stich[i]->dim.h);
                cairo_fill(cr);
            }

            /* draw card owner's name */
            cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
            cairo_select_font_face(cr, "sans-serif",
                    CAIRO_FONT_SLANT_NORMAL,
                    CAIRO_FONT_WEIGHT_BOLD);
            cairo_set_font_size(cr, 12);
            cairo_move_to(cr, x, y + stich[i]->dim.h + 15);
            cairo_show_text(cr, gskat.players[stich[i]->owner]->name);

            x += stich[i]->dim.w + 5;
        }
    }

    cairo_destroy(cr);
    g_free(caption);

    /* end drawing process
     * used for double buffering */
    gdk_window_end_paint(area->window);
}

/* vim:set et sw=4 sts=4 tw=80: */
