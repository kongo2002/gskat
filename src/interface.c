/*
 *  This file is part of gskat.
 *
 *  Copyright (C) 2010-2011 by Gregor Uhlenheuer
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
#include "callback.h"
#include "configuration.h"
#include "common.h"
#include "draw.h"
#include "interface.h"
#include "utils.h"

/**
 * init_player:
 * @id:     player id
 * @name:   player name
 * @human:  %TRUE if player is a human player, otherwise %FALSE
 *
 * Allocate and initialize a new player structure
 *
 * Returns: (transfer full): the new #player object
 */
player *init_player(gint id, gchar *name, gboolean human)
{
    player *new = (player *) g_malloc(sizeof(player));

    new->id = id;
    new->name = name;
    new->human = human;
    new->re = FALSE;
    new->does_bid = FALSE;
    new->points = 0;
    new->sum_points = 0;
    new->round_points = NULL;
    new->gereizt = 0;
    new->cards = NULL;

    return new;
}

/**
 * set_icons:
 *
 * Try to load and set the game icons
 */
void set_icons(void)
{
    gint i;
    gchar *filename;
    GList *icons = NULL;

    const gchar *icon_names[] =
    {
        "gskat16x16.png",
        "gskat22x22.png",
        "gskat24x24.png",
        "gskat32x32.png",
        "gskat48x48.png",
        "gskat64x64.png",
        "gskat128x128.png",
        NULL
    };

    /* fill the icon list with all found icon pixbufs */
    for (i=0; icon_names[i]; ++i)
    {
        filename = g_build_filename(DATA_DIR, "icons", icon_names[i], NULL);

        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(filename, NULL);

        if (pixbuf)
        {
            icons = g_list_prepend(icons, pixbuf);
            gskat_msg(MT_INFO, _("Loaded icon '%s'\n"), filename);
        }

        g_free(filename);
    }

    if (icons)
    {
        gtk_window_set_default_icon_list(icons);

        /* free pixbufs and the list object */
        g_list_foreach(icons, (GFunc) g_object_unref, NULL);
        g_list_free(icons);
    }
}

/**
 * load_suit_icons:
 * @paths:  several possible paths to search the suit icon files in
 *
 * Try to load the icons of the four suits
 */
void load_suit_icons(const gchar **paths)
{
    gint i, j;
    gint maxlen = max_str_lenv(paths);
    gchar *suits[] = { "club", "spade", "heart", "diamond" };
    gchar *filename;

    filename = (gchar *) g_malloc(sizeof(gchar) * (maxlen + 20));

    gskat.icons = (GdkPixbuf **) g_malloc(sizeof(GdkPixbuf *) * 4);

    for (i=0; i<4; i++)
    {
        for (j=0; paths[j]; ++j)
        {
            g_sprintf(filename, "%s/icon-%s.xpm", paths[j], suits[i]);

            if (g_file_test(filename, G_FILE_TEST_EXISTS) == TRUE)
            {
                gskat_msg(MT_INFO, _("Loading '%s' ... OK\n"), filename);
                gskat.icons[i] = gdk_pixbuf_new_from_file(filename, NULL);
                break;
            }
            else
            {
                gskat_msg(MT_ERROR, _("Failed to load icon '%s'\n"), filename);
                gskat.icons[i] = NULL;
            }
        }
    }

    g_free(filename);
}

/**
 * alloc_app:
 *
 * Allocate all game objects like players, icons and tricks array
 */
void alloc_app(void)
{
    gint i;

    /* initialize players */
    gskat.players = (player **) g_malloc(sizeof(player *) * 3);

    for (i=0; i<3; ++i)
        gskat.players[i] = init_player(i, gskat.player_names[i],
                i ? FALSE : TRUE);

    /* initialize played cards */
    gskat.stiche = (trick **) g_malloc(sizeof(trick *) * 10);

    for (i=0; i<10; ++i)
        gskat.stiche[i] = NULL;

    /* initialize alternative cursor shapes */
    gskat.pirate_cursor = gdk_cursor_new(GDK_PIRATE);
    gskat.hand_cursor = gdk_cursor_new(GDK_HAND1);

    gskat_msg(MT_STATUSBAR, _("gskat v%s successfully loaded."), VERSION);
}

/**
 * update_sb:
 * @fmt:   printf-like format-string
 * @args:  additional arguments
 *
 * Remove the current message from the statusbar stack and
 * push a new message onto the message stack
 */
void update_sb(const gchar *fmt, va_list args)
{
    guint i;
    gchar *msg = NULL;
    GtkStatusbar *sb = (GtkStatusbar *) get_widget("statusbar");

    if (sb && fmt)
    {
        msg = g_strdup_vprintf(fmt, args);

        /* remove '\n' from string */
        for (i=0; i < strlen(msg); ++i)
        {
            if (msg[i] == '\n')
                msg[i] = '\x20';
        }

        if (msg)
        {
            gtk_statusbar_pop(sb, GSKAT_CONTEXTID);
            gtk_statusbar_push(sb, GSKAT_CONTEXTID, msg);

            g_free(msg);
        }
    }
}

/**
 * show_dialog:
 * @type:    a #GtkMessageType
 * @buttons: a #GtkButtonsType
 * @fmt:     a printf-like format string
 * @args:    #va_list with optional arguments
 *
 * Show a modal message dialog window with a given @type, @buttons
 * and message.
 */
void show_dialog(GtkMessageType type, GtkButtonsType buttons,
        const gchar *fmt, va_list args)
{
    GtkWidget *dialog;
    gchar *msg = g_strdup_vprintf(fmt, args);

    dialog = gtk_message_dialog_new(GTK_WINDOW(gskat.window),
                GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
                type,
                buttons,
                msg,
                NULL);

    gtk_dialog_run(GTK_DIALOG(dialog));

    gtk_widget_destroy(dialog);
    g_free(msg);
}

/**
 * show_dialog_info:
 * @fmt:  printf-like format string
 * @args: optional arguments
 *
 * Show an info message dialog window with a given message
 */
void show_dialog_info(const gchar *fmt, va_list args)
{
    show_dialog(GTK_MESSAGE_INFO, GTK_BUTTONS_OK, fmt, args);
}

/**
 * show_dialog_error:
 * @fmt:  printf-like format string
 * @args: optional arguments
 *
 * Show an error message dialog window with a given message
 */
void show_dialog_error(const gchar *fmt, va_list args)
{
    show_dialog(GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, fmt, args);
}

/**
 * show_file_bugreport:
 *
 * Show a window to file a new bug report
 */
void show_file_bugreport(void)
{
    gchar *dir;
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *file_frame;
    GtkWidget *file_vbox;
    /* GtkWidget *file_entry; */
    GtkWidget *dir_chooser;
    GtkWidget *desc_frame;
    GtkWidget *desc_vbox;
    GtkWidget *desc_text_view;
    GtkTextBuffer *desc_text_buffer;
    GtkWidget *hbox_button;
    GtkWidget *ok_button;
    GtkWidget *cancel_button;
    GtkWidget *info_label;
    br_group *bug_report_group;

    /* determine desktop directory */
    dir = get_desktop_dir();

    /* create window widget */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(window, 300, 300);
    gtk_window_set_title(GTK_WINDOW(window), _("File bug report"));
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_set_transient_for(GTK_WINDOW(window),
            GTK_WINDOW(gskat.window));

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    /* file name frame widget */
    file_frame = gtk_frame_new(_("Save bug report into:"));
    gtk_box_pack_start(GTK_BOX(vbox), file_frame, FALSE, FALSE, 2);

    file_vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(file_vbox), 5);
    gtk_container_add(GTK_CONTAINER(file_frame), file_vbox);

    dir_chooser = gtk_file_chooser_button_new(_("Choose directory"),
            GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dir_chooser), dir);
    gtk_box_pack_start(GTK_BOX(file_vbox), dir_chooser, FALSE, FALSE, 2);

    /*
    file_entry = gtk_entry_new();
    gtk_widget_set_sensitive(file_entry, FALSE);
    gtk_entry_set_text(GTK_ENTRY(file_entry), filename);
    gtk_box_pack_start(GTK_BOX(file_vbox), file_entry, TRUE, TRUE, 2);
    */

    /* description frame widget */
    desc_frame = gtk_frame_new(_("Description:"));
    gtk_box_pack_start(GTK_BOX(vbox), desc_frame, TRUE, TRUE, 2);

    desc_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(desc_vbox), 5);
    gtk_container_add(GTK_CONTAINER(desc_frame), desc_vbox);

    /* description text view */
    desc_text_view = gtk_text_view_new();
    desc_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(desc_text_view));
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(desc_text_view), 5);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(desc_text_view), 5);
    gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(desc_text_view), 5);
    gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(desc_text_view), 5);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(desc_text_view), GTK_WRAP_WORD);
    gtk_box_pack_start(GTK_BOX(desc_vbox), desc_text_view, TRUE, TRUE, 2);

    /* information/send label */
    info_label = gtk_label_new(NULL);
    gtk_label_set_selectable(GTK_LABEL(info_label), TRUE);
    gtk_label_set_line_wrap(GTK_LABEL(info_label), TRUE);
    gtk_label_set_line_wrap_mode(GTK_LABEL(info_label), PANGO_WRAP_WORD);

#if GTK_CHECK_VERSION(2, 18, 0)
    gtk_label_set_markup(GTK_LABEL(info_label),
            _("Please send this bug report to\n<a href=\"mailto:"
                "kongo2002@googlemail.com\" title=\"Send mail\">"
                "kongo2002@googlemail.com</a>"));
#else
    gtk_label_set_markup(GTK_LABEL(info_label),
            _("Please send this bug report to\n"
                "kongo2002@googlemail.com"));
#endif

    gtk_box_pack_start(GTK_BOX(vbox), info_label, FALSE, FALSE, 2);

    /* buttons */
    hbox_button = gtk_hbutton_box_new();
    gtk_container_set_border_width(GTK_CONTAINER(hbox_button), 5);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox_button),
            GTK_BUTTONBOX_SPREAD);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_button, FALSE, FALSE, 2);

    ok_button = gtk_button_new_from_stock(GTK_STOCK_OK);
    gtk_container_add(GTK_CONTAINER(hbox_button), ok_button);

    cancel_button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    gtk_container_add(GTK_CONTAINER(hbox_button), cancel_button);

    /* initialize new bug report structure */
    bug_report_group = (br_group *) g_malloc(sizeof(br_group));

    bug_report_group->window = window;
    bug_report_group->file_chooser = dir_chooser;
    bug_report_group->text_buffer = desc_text_buffer;

    /* connect signals */
    g_signal_connect(G_OBJECT(ok_button), "clicked",
            G_CALLBACK(save_bugreport), (gpointer) bug_report_group);
    g_signal_connect(G_OBJECT(cancel_button), "clicked",
            G_CALLBACK(close_bugreport), (gpointer) bug_report_group);
    g_signal_connect(G_OBJECT(window), "delete-event",
            G_CALLBACK(destroy_bugreport), (gpointer) bug_report_group);

    gtk_widget_show_all(window);
}

/**
 * show_bid_infobar:
 * @value: Bid value
 * @msg:   String containing the message for the #GtkInfoBar
 * @hoeren: User has to 'hear'?
 *
 * Shows a new #GtkInfoBar widget on the main game interface.
 * The callback functions return the respective value the player
 * wants to bid.
 */
void show_bid_infobar(gint value, gchar *msg, gboolean hoeren)
{
    gchar caption[4];
    GtkWidget *infobar;
    GtkWidget *content;
    GtkWidget *message;

    g_sprintf(caption, "%d", value);

    infobar = gtk_info_bar_new_with_buttons((hoeren) ? _("Yes") : caption,
            value, (hoeren) ? _("No") : _("Pass"), 0, NULL);
    gtk_info_bar_set_message_type(GTK_INFO_BAR(infobar), GTK_MESSAGE_QUESTION);

    g_signal_connect(infobar, "response", G_CALLBACK(infobar_bid_response),
            GINT_TO_POINTER(hoeren));

    message = gtk_label_new(msg);

    content = gtk_info_bar_get_content_area(GTK_INFO_BAR(infobar));
    gtk_container_add(GTK_CONTAINER(content), message);

    gtk_table_attach_defaults(GTK_TABLE(get_widget("vtable")),
            infobar,
            0, 1, 0, 1);

    gtk_widget_show_all(infobar);
}

/**
 * add_summary:
 * @tree:   #GtkTreeStore that contains the game summary information
 * @name:   name of the new row (first column)
 * @value:  value of the new row (second column)
 *
 * Append a new line into the game summary tree view.
 */
static void add_summary(GtkTreeStore **tree, const char *name, const char *value)
{
    GtkTreeIter child;

    /* add a new row to the tree view */
    gtk_tree_store_append(*tree, &child, NULL);

    /* set the new rows' values */
    gtk_tree_store_set(*tree, &child,
            0, name, 1, value, -1);
}

/**
 * add_summary_row:
 * @tree:   #GtkTreeStore that contains the game summary information
 * @name:   name of the new row (first column)
 * @value:  value of the new row (second column)
 *
 * Append a new line into the game summary tree view.
 */
void add_summary_row(GtkTreeStore **tree, const char *name, const char *value)
{
    add_summary(tree, name, value);
}

/**
 * add_summary_row_int:
 * @tree:   #GtkTreeStore that contains the game summary information
 * @name:   name of the new row (first column)
 * @value:  integer value of the new row (second column)
 *
 * Append a new line into the game summary tree view.
 */
void add_summary_row_int(GtkTreeStore **tree, const char *name, gint value)
{
    gchar *string;

    string = g_strdup_printf("%d", value);

    add_summary(tree, name, string);

    g_free(string);
}

/**
 * create_game_summary:
 * @tree:   #GtkTreeStore object that (will) contains the information
 *
 * Create a game summary dialog window and display all statistics of the
 * finished round.
 *
 * Returns: (transfer full): a new #GtkWindow instance that contains the
 * newly created game summary
 */
GtkWidget *create_game_summary(GtkTreeStore **tree)
{
    gchar str[64];

    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *table;
    GtkWidget *button;
    GtkWidget *player_name;
    GtkWidget *player_name_value;
    GtkWidget *bidden;
    GtkWidget *bidden_value;
    GtkWidget *game;
    GtkWidget *game_value;
    GtkWidget *points;
    GtkWidget *points_value;
    GtkWidget *scrolledwin;

    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    GtkWidget *view;
    GtkTreeModel *model;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), _("Game summary"));
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_set_transient_for(GTK_WINDOW(window),
            GTK_WINDOW(gskat.window));
    gtk_widget_set_size_request(window, 300, 300);

    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    /* CREATE TABLE HEADER */
    table = gtk_table_new(4, 2, TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(table), 5);

    /* player name */
    player_name = gtk_label_new(_("Player:"));
    player_name_value = gtk_label_new(gskat.re->name);

    gtk_misc_set_alignment(GTK_MISC(player_name), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table),
            player_name,
            0, 1, 0, 1);

    gtk_misc_set_alignment(GTK_MISC(player_name_value), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table),
            player_name_value,
            1, 2, 0, 1);

    /* bidden value */
    bidden = gtk_label_new(_("Bid value:"));

    g_snprintf(str, 64, "%d", gskat.bidden);
    bidden_value = gtk_label_new(str);

    gtk_misc_set_alignment(GTK_MISC(bidden), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table),
            bidden,
            0, 1, 1, 2);

    gtk_misc_set_alignment(GTK_MISC(bidden_value), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table),
            bidden_value,
            1, 2, 1, 2);

    /* game name */
    game = gtk_label_new(_("Game:"));
    game_value = gtk_label_new(get_game_name());

    gtk_misc_set_alignment(GTK_MISC(game), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table),
            game,
            0, 1, 2, 3);

    gtk_misc_set_alignment(GTK_MISC(game_value), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table),
            game_value,
            1, 2, 2, 3);

    /* player's points */
    points = gtk_label_new(_("Points:"));
    points_value = gtk_label_new(NULL);

    g_snprintf(str, 64, "<span font_weight=\"bold\">%d</span> of 120 points",
            gskat.re->points);
    gtk_label_set_markup(GTK_LABEL(points_value), str);

    gtk_misc_set_alignment(GTK_MISC(points), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table),
            points,
            0, 1, 3, 4);

    gtk_misc_set_alignment(GTK_MISC(points_value), 0, 0.5);
    gtk_table_attach_defaults(GTK_TABLE(table),
            points_value,
            1, 2, 3, 4);

    gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 2);

    /* CREATE TREE VIEW */
    view = gtk_tree_view_new();

    /* first column */
    column = gtk_tree_view_column_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

    renderer = gtk_cell_renderer_text_new();

    gtk_tree_view_column_pack_start(column, renderer, TRUE);

    gtk_tree_view_column_add_attribute(column, renderer, "text", 0);

    /* first column in bold font */
    g_object_set(renderer,
            "weight", PANGO_WEIGHT_BOLD,
            "weight-set", TRUE,
            NULL);

    /* second column */
    column = gtk_tree_view_column_new();
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

    renderer = gtk_cell_renderer_text_new();

    gtk_tree_view_column_pack_start(column, renderer, TRUE);

    gtk_tree_view_column_add_attribute(column, renderer, "text", 1);

    /* CREATE THE TREE MODEL */
    *tree = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

    model = GTK_TREE_MODEL(*tree);
    gtk_tree_view_set_model(GTK_TREE_VIEW(view), model);
    gtk_tree_selection_set_mode(gtk_tree_view_get_selection(
                GTK_TREE_VIEW(view)), GTK_SELECTION_NONE);

    /* SCROLLED WINDOW */
    scrolledwin = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwin),
            GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

    gtk_container_add(GTK_CONTAINER(scrolledwin), view);

    gtk_box_pack_start(GTK_BOX(vbox), scrolledwin, TRUE, TRUE, 2);

    button = gtk_button_new_from_stock(GTK_STOCK_OK);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 2);

    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(window_close),
            window);

    return window;
}

/**
 * show_last_tricks:
 *
 * Show a dialog window showing the last trick(s)
 */
void show_last_tricks(void)
{
    gint cur, x, y;
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *area;
    GtkWidget *hsep;
    GtkWidget *hbox_button;
    GtkWidget *prev_button;
    GtkWidget *next_button;
    GtkWidget *button;
    trick *stich;
    stich_view *sv = NULL;

    cur = gskat.stich - 2;

    /* return if there is no stich to show */
    if (cur < 0 || (stich = gskat.stiche[cur]) == NULL)
        return;

    /* get minimal drawing area size to request */
    x = ((gskat.stiche[0]->cards[0])->dim.w + 5) * 3 + 5;
    y = (gskat.stiche[0]->cards[0])->dim.h + 80;

    /* dialog window widgets */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), _("Previous trick"));
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_set_transient_for(GTK_WINDOW(window),
            GTK_WINDOW(gskat.window));

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    /* drawing area */
    area = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), area, TRUE, TRUE, 2);
    gtk_widget_set_size_request(area, x, y);
    gtk_widget_set_double_buffered(area, TRUE);

    hsep = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), hsep, FALSE, FALSE, 0);

    hbox_button = gtk_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_button, FALSE, FALSE, 2);

    /* previous stich button */
    prev_button = gtk_button_new_from_stock(GTK_STOCK_GO_BACK);
    gtk_box_pack_start(GTK_BOX(hbox_button), prev_button, FALSE, FALSE, 2);

    /* deactivate previous button if the first stich is already shown
     * or if 'show_tricks' is turned off
     * or if 'num_show_tricks' == 1 */
    if (cur == 0 || !get_prop_bool("show_tricks") ||
            get_prop_int("num_show_tricks") <= 1)
        gtk_widget_set_sensitive(prev_button, FALSE);

    /* close/ok button */
    button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
    gtk_box_pack_start(GTK_BOX(hbox_button), button, FALSE, FALSE, 2);

    /* next stich button (initially deactivated) */
    next_button = gtk_button_new_from_stock(GTK_STOCK_GO_FORWARD);
    gtk_box_pack_start(GTK_BOX(hbox_button), next_button, FALSE, FALSE, 2);
    gtk_widget_set_sensitive(next_button, FALSE);

    /* initialize stich_view structure */
    sv = g_malloc(sizeof(stich_view));

    sv->cur = cur;
    sv->stich = stich;
    sv->window = window;
    sv->area = area;
    sv->prevb = prev_button;
    sv->nextb = next_button;

    /* connect signals with callback functions */
    g_signal_connect(G_OBJECT(button), "clicked",
            G_CALLBACK(close_show_trick), (gpointer) sv);
    g_signal_connect(G_OBJECT(prev_button), "clicked",
            G_CALLBACK(prev_stich_click), (gpointer) sv);
    g_signal_connect(G_OBJECT(next_button), "clicked",
            G_CALLBACK(next_stich_click), (gpointer) sv);
    g_signal_connect(G_OBJECT(area), "expose-event",
            G_CALLBACK(refresh_tricks), (gpointer) sv);
    g_signal_connect(G_OBJECT(window), "delete-event",
            G_CALLBACK(destroy_show_trick), (gpointer) sv);

    gtk_widget_show_all(window);

    draw_tricks_area(area, sv);
}

/**
 * show_config_window:
 *
 * Show the configuration dialog window and initialize the
 * widgets with the current config values
 */
void show_config_window(void)
{
    gint i;
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *notebook;

    GtkWidget *names_label;
    GtkWidget *names_table;
    GtkWidget *player_label[3];
    GtkWidget *player_entry[3];

    GtkWidget *misc_label;
    GtkWidget *misc_table;
    GtkWidget *debug_label;
    GtkWidget *debug_check;

    GtkWidget *graphics_label;
    GtkWidget *graphics_table;
    GtkWidget *animation_label;
    GtkWidget *animation_check;
    GtkWidget *animation_dur_label;
    GtkWidget *animation_duration;
    GtkWidget *reaction_label;
    GtkWidget *reaction_check;
    GtkWidget *reaction_dur_label;
    GtkWidget *reaction_duration;
    GtkWidget *provoke_values_label;
    GtkWidget *provoke_values;

    GtkWidget *rules_label;
    GtkWidget *rules_table;
    GtkWidget *show_tricks_label;
    GtkWidget *show_tricks_check;
    GtkWidget *num_show_tricks_label;
    GtkWidget *num_show_tricks;
    GtkWidget *show_poss_label;
    GtkWidget *show_poss_check;

    gint about_entry_len;
    GtkWidget *about_label;
    GtkWidget *about_table;
    GtkWidget *config_loc_label;
    GtkWidget *config_loc_entry;
    GtkWidget *data_loc_label;
    GtkWidget *data_loc_entry;
    GtkWidget *locale_loc_label;
    GtkWidget *locale_loc_entry;

    GtkWidget *hbox_buttons;
    GtkWidget *ok_button;
    GtkWidget *cancel_button;

    GtkWidget **confwidgets;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), _("Properties"));
    gtk_window_set_modal(GTK_WINDOW(window), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(window),
            GTK_WINDOW(gskat.window));

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_LEFT);
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

    /* PLAYER NAMES TABLE */
    names_label = gtk_label_new(_("Player names"));
    names_table = gtk_table_new(3, 2, TRUE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), names_table, names_label);
    gtk_container_set_border_width(GTK_CONTAINER(names_table), 5);

    player_label[0] = gtk_label_new(_("Your name:"));
    player_label[1] = gtk_label_new(_("Player 1:"));
    player_label[2] = gtk_label_new(_("Player 2:"));

    for (i=0; i<3; ++i)
    {
        gtk_misc_set_alignment(GTK_MISC(player_label[i]), 0, 0.5);
        gtk_table_attach_defaults(GTK_TABLE(names_table),
                player_label[i],
                0, 1, i, i+1);

        player_entry[i] = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(player_entry[i]),
                gskat.player_names[i]);
        gtk_table_attach_defaults(GTK_TABLE(names_table),
                player_entry[i],
                1, 2, i, i+1);
    }

    /* RULES TABLE */
    rules_label = gtk_label_new(_("Rules"));
    rules_table = gtk_table_new(3, 2, FALSE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), rules_table, rules_label);
    gtk_container_set_border_width(GTK_CONTAINER(rules_table), 5);

    show_tricks_label = gtk_label_new(_("Show previous trick(s):"));
    gtk_misc_set_alignment(GTK_MISC(show_tricks_label), 0, 0.5);

    show_tricks_check = gtk_check_button_new();
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_tricks_check),
            get_prop_bool("show_tricks"));
    gtk_table_attach(GTK_TABLE(rules_table),
            show_tricks_label,
            0, 1, 0, 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(rules_table),
            show_tricks_check,
            1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 10, 0);

    num_show_tricks_label = gtk_label_new(_("Number of reviewable tricks:"));
    gtk_misc_set_alignment(GTK_MISC(num_show_tricks_label), 0, 0.5);

    num_show_tricks = gtk_spin_button_new_with_range(1, 11, 1.0);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(num_show_tricks), 0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(num_show_tricks),
            get_prop_int("num_show_tricks"));
    gtk_widget_set_sensitive(num_show_tricks, get_prop_bool("show_tricks"));

    gtk_table_attach(GTK_TABLE(rules_table),
            num_show_tricks_label,
            0, 1, 1, 2, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(rules_table),
            num_show_tricks,
            1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 10, 0);

    g_signal_connect(G_OBJECT(show_tricks_check), "toggled",
            G_CALLBACK(show_tricks_toggle), num_show_tricks);

    show_poss_label = gtk_label_new(_("Change mouse cursor for possible cards:"));
    gtk_misc_set_alignment(GTK_MISC(show_poss_label), 0, 0.5);

    show_poss_check = gtk_check_button_new();
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_poss_check),
            get_prop_bool("show_poss_cards"));

    gtk_table_attach(GTK_TABLE(rules_table),
            show_poss_label,
            0, 1, 2, 3, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(rules_table),
            show_poss_check,
            1, 2, 2, 3, GTK_SHRINK, GTK_SHRINK, 10, 0);

    /* GRAPHICS TABLE */
    graphics_label = gtk_label_new(_("Graphics"));
    graphics_table = gtk_table_new(5, 2, FALSE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), graphics_table,
            graphics_label);
    gtk_container_set_border_width(GTK_CONTAINER(graphics_table), 5);

    /* animation */
    animation_label = gtk_label_new(_("Animate card movement:"));
    gtk_misc_set_alignment(GTK_MISC(animation_label), 0, 0.5);
    gtk_table_attach(GTK_TABLE(graphics_table),
            animation_label,
            0, 1, 0, 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

    animation_check = gtk_check_button_new();
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(animation_check),
            get_prop_bool("animation"));
    gtk_table_attach(GTK_TABLE(graphics_table),
            animation_check,
            1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 10, 0);

    /* animation duration */
    animation_dur_label = gtk_label_new(_("Animation duration (in ms):"));
    gtk_misc_set_alignment(GTK_MISC(animation_dur_label), 0, 0.5);
    gtk_table_attach(GTK_TABLE(graphics_table),
            animation_dur_label,
            0, 1, 1, 2, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

    animation_duration = gtk_spin_button_new_with_range(25, 5000, 10.0);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(animation_duration), 0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(animation_duration),
            get_prop_int("anim_duration"));
    gtk_widget_set_sensitive(animation_duration, get_prop_bool("animation"));
    gtk_table_attach(GTK_TABLE(graphics_table),
            animation_duration,
            1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 10, 0);

    g_signal_connect(G_OBJECT(animation_check), "toggled",
            G_CALLBACK(animation_toggle), animation_duration);

    /* opponents reaction */
    reaction_label = gtk_label_new(_("Delay opponents reaction:"));
    gtk_misc_set_alignment(GTK_MISC(reaction_label), 0, 0.5);
    gtk_table_attach(GTK_TABLE(graphics_table),
            reaction_label,
            0, 1, 2, 3, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

    reaction_check = gtk_check_button_new();
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(reaction_check),
            get_prop_bool("reaction"));
    gtk_table_attach(GTK_TABLE(graphics_table),
            reaction_check,
            1, 2, 2, 3, GTK_SHRINK, GTK_SHRINK, 10, 0);

    /* reaction duration */
    reaction_dur_label = gtk_label_new(_("Reaction time (in ms):"));
    gtk_misc_set_alignment(GTK_MISC(reaction_dur_label), 0, 0.5);
    gtk_table_attach(GTK_TABLE(graphics_table),
            reaction_dur_label,
            0, 1, 3, 4, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

    reaction_duration = gtk_spin_button_new_with_range(100, 5000, 20.0);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(reaction_duration), 0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(reaction_duration),
            get_prop_int("reaction_duration"));
    gtk_widget_set_sensitive(reaction_duration, get_prop_bool("reaction"));
    gtk_table_attach(GTK_TABLE(graphics_table),
            reaction_duration,
            1, 2, 3, 4, GTK_SHRINK, GTK_SHRINK, 10, 0);

    g_signal_connect(G_OBJECT(reaction_check), "toggled",
            G_CALLBACK(reaction_toggle), reaction_duration);

    /* provoke values */
    provoke_values_label = gtk_label_new(_("Show players' provoke values:"));
    gtk_misc_set_alignment(GTK_MISC(provoke_values_label), 0, 0.5);
    gtk_table_attach(GTK_TABLE(graphics_table),
            provoke_values_label,
            0, 1, 4, 5, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

    provoke_values = gtk_check_button_new();
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(provoke_values),
            get_prop_bool("provoke_values"));
    gtk_table_attach(GTK_TABLE(graphics_table),
            provoke_values,
            1, 2, 4, 5, GTK_SHRINK, GTK_SHRINK, 10, 0);

    /* MISC TABLE */
    misc_label = gtk_label_new(_("Misc"));
    misc_table = gtk_table_new(1, 2, FALSE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), misc_table,
            misc_label);
    gtk_container_set_border_width(GTK_CONTAINER(misc_table), 5);

    /* debugging */
    debug_label = gtk_label_new(_("Print debug statements:"));
    gtk_misc_set_alignment(GTK_MISC(debug_label), 0, 0.5);
    gtk_table_attach(GTK_TABLE(misc_table),
            debug_label,
            0, 1, 0, 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

    debug_check = gtk_check_button_new();
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(debug_check),
            get_prop_bool("debug"));
    gtk_table_attach(GTK_TABLE(misc_table),
            debug_check,
            1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 10, 0);

#ifdef DEBUG
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(debug_check), TRUE);

    gtk_widget_set_sensitive(debug_label, FALSE);
    gtk_widget_set_sensitive(debug_check, FALSE);
#endif

    /* ABOUT TABLE */
    about_label = gtk_label_new(_("About"));
    about_table = gtk_table_new(3, 2, FALSE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), about_table, about_label);
    gtk_container_set_border_width(GTK_CONTAINER(about_table), 5);

    about_entry_len = max_str_len(get_config_dir(), DATA_DIR,
            GSKAT_LOCALEDIR, NULL);

    config_loc_label = gtk_label_new(_("Configuration file location:"));
    gtk_misc_set_alignment(GTK_MISC(config_loc_label), 0, 0.5);
    gtk_table_attach(GTK_TABLE(about_table),
            config_loc_label,
            0, 1, 0, 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

    config_loc_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(config_loc_entry), get_config_dir());
    gtk_entry_set_width_chars(GTK_ENTRY(config_loc_entry), about_entry_len);
    gtk_editable_set_editable(GTK_EDITABLE(config_loc_entry), FALSE);
    gtk_table_attach(GTK_TABLE(about_table),
            config_loc_entry,
            1, 2, 0, 1, GTK_SHRINK, GTK_SHRINK, 10, 0);

    data_loc_label = gtk_label_new(_("Resource files location:"));
    gtk_misc_set_alignment(GTK_MISC(data_loc_label), 0, 0.5);
    gtk_table_attach(GTK_TABLE(about_table),
            data_loc_label,
            0, 1, 1, 2, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

    data_loc_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(data_loc_entry), DATA_DIR);
    gtk_entry_set_width_chars(GTK_ENTRY(data_loc_entry), about_entry_len);
    gtk_editable_set_editable(GTK_EDITABLE(data_loc_entry), FALSE);
    gtk_table_attach(GTK_TABLE(about_table),
            data_loc_entry,
            1, 2, 1, 2, GTK_SHRINK, GTK_SHRINK, 10, 0);

    locale_loc_label = gtk_label_new(_("Localization files location:"));
    gtk_misc_set_alignment(GTK_MISC(locale_loc_label), 0, 0.5);
    gtk_table_attach(GTK_TABLE(about_table),
            locale_loc_label,
            0, 1, 2, 3, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);

    locale_loc_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(locale_loc_entry), GSKAT_LOCALEDIR);
    gtk_entry_set_width_chars(GTK_ENTRY(locale_loc_entry), about_entry_len);
    gtk_editable_set_editable(GTK_EDITABLE(locale_loc_entry), FALSE);
    gtk_table_attach(GTK_TABLE(about_table),
            locale_loc_entry,
            1, 2, 2, 3, GTK_SHRINK, GTK_SHRINK, 10, 0);

    /* BOTTOM BUTTONS */
    hbox_buttons = gtk_hbox_new(TRUE, 10);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_buttons, FALSE, FALSE, 0);

    ok_button = gtk_button_new_from_stock(GTK_STOCK_APPLY);
    gtk_box_pack_start(GTK_BOX(hbox_buttons), ok_button, FALSE, FALSE, 0);

    cancel_button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    gtk_box_pack_start(GTK_BOX(hbox_buttons), cancel_button, FALSE, FALSE, 0);

    /* allocate configuration widgets
     * this array is freed in either 'save_config', 'close_config'
     * or 'destroy_config' */
    confwidgets = (GtkWidget **) g_malloc(4 * sizeof(GtkWidget *));

    confwidgets[0] = player_entry[0];
    confwidgets[1] = player_entry[1];
    confwidgets[2] = player_entry[2];
    confwidgets[3] = window;

    set_prop_widget("animation", animation_check);
    set_prop_widget("anim_duration", animation_duration);
    set_prop_widget("debug", debug_check);
    set_prop_widget("show_tricks", show_tricks_check);
    set_prop_widget("num_show_tricks", num_show_tricks);
    set_prop_widget("show_poss_cards", show_poss_check);
    set_prop_widget("reaction", reaction_check);
    set_prop_widget("reaction_duration", reaction_duration);
    set_prop_widget("provoke_values", provoke_values);

    g_signal_connect(G_OBJECT(ok_button), "clicked",
            G_CALLBACK(save_config), (gpointer) confwidgets);
    g_signal_connect(G_OBJECT(cancel_button), "clicked",
            G_CALLBACK(close_config), (gpointer) confwidgets);
    g_signal_connect(G_OBJECT(window), "delete-event",
            G_CALLBACK(destroy_config), (gpointer) confwidgets);

    gtk_widget_show_all(window);
}

/**
 * create_menu:
 *
 * Create the main menu and populate it with the menu items
 *
 * Returns: (transfer full): the new main menu #GtkWidget
 */
static GtkWidget *create_menu()
{
    GtkWidget *menu;         /* main menu */
    GtkWidget *gmenu;        /* game submenu */
    GtkWidget *game;
    GtkWidget *new_item;
    GtkWidget *gameload_item;
    GtkWidget *gameload_image;
    GtkWidget *gamesave_item;
    GtkWidget *gamesave_image;
    GtkWidget *quickload_item;
    GtkWidget *quicksave_item;
    GtkWidget *quit_item;
    GtkWidget *cmenu;        /* configuration submenu */
    GtkWidget *config;
    GtkWidget *options_item;
    GtkWidget *brmenu;       /* bug report submenu */
    GtkWidget *bugreport;
    GtkWidget *bugreport_item;
    GtkWidget *hmenu;        /* help submenu */
    GtkWidget *help;
    GtkWidget *about_item;

    menu = gtk_menu_bar_new();

    /* game submenu */
    new_item = gtk_menu_item_new_with_label(_("New round"));
    g_signal_connect(G_OBJECT(new_item), "activate",
            G_CALLBACK(next_round), NULL);

    gameload_item = gtk_image_menu_item_new_with_label(_("Load game ..."));
    gameload_image = gtk_image_new_from_stock(GTK_STOCK_OPEN,
            GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(gameload_item),
            gameload_image);
    g_signal_connect(G_OBJECT(gameload_item), "activate",
            G_CALLBACK(gameload_cb), NULL);

    gamesave_item = gtk_image_menu_item_new_with_label(_("Save game ..."));
    gamesave_image = gtk_image_new_from_stock(GTK_STOCK_SAVE,
            GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(gamesave_item),
            gamesave_image);
    gtk_widget_set_sensitive(gamesave_item, FALSE);
    g_signal_connect(G_OBJECT(gamesave_item), "activate",
            G_CALLBACK(gamesave_cb), NULL);

    quickload_item = gtk_menu_item_new_with_label(_("Quick load"));
    g_signal_connect(G_OBJECT(quickload_item), "activate",
            G_CALLBACK(quickload_game_cb), NULL);
    quicksave_item = gtk_menu_item_new_with_label(_("Quick save"));
    gtk_widget_set_sensitive(quicksave_item, FALSE);
    g_signal_connect(G_OBJECT(quicksave_item), "activate",
            G_CALLBACK(quicksave_game_cb), NULL);
    quit_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
    g_signal_connect(G_OBJECT(quit_item), "activate", G_CALLBACK(quit), NULL);

    gmenu = gtk_menu_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(gmenu), new_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(gmenu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(gmenu), gameload_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(gmenu), gamesave_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(gmenu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(gmenu), quickload_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(gmenu), quicksave_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(gmenu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(gmenu), quit_item);

    game = gtk_menu_item_new_with_label(_("Game"));
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(game), gmenu);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), game);

    /* configuration submenu */
    options_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES, NULL);
    g_signal_connect(G_OBJECT(options_item), "activate",
            G_CALLBACK(show_config_window), NULL);

    cmenu = gtk_menu_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(cmenu), options_item);

    config = gtk_menu_item_new_with_label(_("Preferences"));
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(config), cmenu);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), config);

    /* bug report submenu */
    bugreport_item = gtk_menu_item_new_with_label(_("File bug report"));
    gtk_widget_set_sensitive(bugreport_item, FALSE);
    g_signal_connect(G_OBJECT(bugreport_item), "activate",
            G_CALLBACK(show_file_bugreport), NULL);

    brmenu = gtk_menu_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(brmenu), bugreport_item);

    bugreport = gtk_menu_item_new_with_label(_("Bug report"));
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(bugreport), brmenu);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), bugreport);

    /* help submenu */
    about_item = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
    g_signal_connect(G_OBJECT(about_item), "activate",
            G_CALLBACK(show_about_window), NULL);

    hmenu = gtk_menu_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(hmenu), about_item);

    help = gtk_menu_item_new_with_label(_("Help"));
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), hmenu);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), help);

    /* add necessary menu item widgets to hash table */
    g_hash_table_insert(gskat.widgets, "mi_quicksave", quicksave_item);
    g_hash_table_insert(gskat.widgets, "mi_bugreport", bugreport_item);
    g_hash_table_insert(gskat.widgets, "mi_gamesave", gamesave_item);

    return menu;
}

/**
 * create_interface:
 *
 * Create and allocate the main window layout
 */
void create_interface(void)
{
    GtkWidget *window;
    GtkWidget *vboxmenu;
    GtkWidget *mainmenu;
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *vtable;
    GtkWidget *area;
    GtkWidget *statusbar;
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
    GtkWidget *scrolled_win;
    GtkWidget *vbox_table;
    GtkWidget *table_rank;
    GtkWidget *table_points;
    GtkWidget *hsep;
    GtkWidget *lb_rank_p1_name;
    GtkWidget *lb_rank_p2_name;
    GtkWidget *lb_rank_p3_name;
    GtkWidget *lb_rank_p1;
    GtkWidget *lb_rank_p2;
    GtkWidget *lb_rank_p3;
    GtkWidget *button;

    gchar *iconfile = (gchar *) g_malloc(sizeof(gchar) * strlen(DATA_DIR)+20);

    g_sprintf(iconfile, "%s/gskat.png", DATA_DIR);

    gskat.widgets = g_hash_table_new(g_str_hash, g_str_equal);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "gskat");

    if (g_file_test(iconfile, G_FILE_TEST_EXISTS))
        gtk_window_set_icon_from_file(GTK_WINDOW(window), iconfile, NULL);

    g_free(iconfile);

    vboxmenu = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window), vboxmenu);

    mainmenu = create_menu(window);
    if (mainmenu)
        gtk_box_pack_start(GTK_BOX(vboxmenu), mainmenu, FALSE, FALSE, 0);

    hbox = gtk_hbox_new(FALSE, 0);
    /* gtk_box_pack_start(child, expand, fill, padding) */
    gtk_box_pack_start(GTK_BOX(vboxmenu), hbox, TRUE, TRUE, 0);

    statusbar = gtk_statusbar_new();
    gtk_box_pack_start(GTK_BOX(vboxmenu), statusbar, FALSE, TRUE, 0);

    vtable = gtk_table_new(2, 1, FALSE);
    gtk_box_pack_start(GTK_BOX(hbox), vtable, TRUE, TRUE, 2);

    area = gtk_drawing_area_new();
    gtk_widget_set_size_request(area, 450, 500);
    gtk_widget_set_double_buffered(area, TRUE);

    gtk_table_attach_defaults(GTK_TABLE(vtable),
            area,
            0, 1, 1, 2);

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, TRUE, 2);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);

    frame_game = gtk_frame_new(_("Round 1"));
    gtk_frame_set_label_align(GTK_FRAME(frame_game), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), frame_game, FALSE, TRUE, 2);
    gtk_frame_set_shadow_type(GTK_FRAME(frame_game), GTK_SHADOW_ETCHED_IN);

    /* gtk_table_new(rows, columns, homogeneous) */
    table_game = gtk_table_new(4, 2, FALSE);
    gtk_container_add(GTK_CONTAINER(frame_game), table_game);
    gtk_container_set_border_width(GTK_CONTAINER(table_game), 20);
    gtk_table_set_col_spacings(GTK_TABLE(table_game), 20);
    gtk_table_set_row_spacings(GTK_TABLE(table_game), 5);

    lb_game_stich_left = gtk_label_new(_("Trick:"));
    lb_game_re_left = gtk_label_new(_("Re:"));
    lb_game_spiel_left = gtk_label_new(_("Game:"));
    lb_game_gereizt_left = gtk_label_new(_("Bidden:"));

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

    /* game rankings */
    frame_rank = gtk_frame_new(_("Rankings"));
    gtk_frame_set_label_align(GTK_FRAME(frame_rank), 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), frame_rank, TRUE, TRUE, 2);
    gtk_frame_set_shadow_type(GTK_FRAME(frame_rank), GTK_SHADOW_ETCHED_IN);

    scrolled_win = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_set_border_width(GTK_CONTAINER(scrolled_win), 5);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win),
            GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolled_win),
            GTK_SHADOW_NONE);
    gtk_container_add(GTK_CONTAINER(frame_rank), scrolled_win);

    vbox_table = gtk_vbox_new(FALSE, 2);
    gtk_container_set_border_width(GTK_CONTAINER(vbox_table), 5);

    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_win),
            vbox_table);

    /* players' points table */
    table_rank = gtk_table_new(1, 3, TRUE);
    gtk_box_pack_start(GTK_BOX(vbox_table), table_rank, FALSE, TRUE, 2);
    gtk_container_set_border_width(GTK_CONTAINER(table_rank), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table_rank), 10);
    gtk_table_set_row_spacings(GTK_TABLE(table_rank), 5);

    lb_rank_p1_name = gtk_label_new(gskat.player_names[0]);
    lb_rank_p2_name = gtk_label_new(gskat.player_names[1]);
    lb_rank_p3_name = gtk_label_new(gskat.player_names[2]);

    gtk_table_attach_defaults(GTK_TABLE(table_rank),
            lb_rank_p1_name,
            0, 1, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table_rank),
            lb_rank_p2_name,
            1, 2, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table_rank),
            lb_rank_p3_name,
            2, 3, 0, 1);

    hsep = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox_table), hsep, FALSE, TRUE, 0);

    table_points = gtk_table_new(1, 3, TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(table_points), 5);
    gtk_table_set_col_spacings(GTK_TABLE(table_points), 10);
    gtk_table_set_row_spacings(GTK_TABLE(table_points), 0);

    lb_rank_p1 = gtk_label_new("");
    lb_rank_p2 = gtk_label_new("");
    lb_rank_p3 = gtk_label_new("");

    gtk_label_set_markup(GTK_LABEL(lb_rank_p1), "<b>0</b>");
    gtk_label_set_markup(GTK_LABEL(lb_rank_p2), "<b>0</b>");
    gtk_label_set_markup(GTK_LABEL(lb_rank_p3), "<b>0</b>");

    gtk_table_attach_defaults(GTK_TABLE(table_points),
            lb_rank_p1,
            0, 1, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table_points),
            lb_rank_p2,
            1, 2, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table_points),
            lb_rank_p3,
            2, 3, 0, 1);

    gtk_box_pack_start(GTK_BOX(vbox_table), table_points, FALSE, TRUE, 2);

    button = gtk_button_new_with_label(_("New Round"));
    gtk_widget_set_sensitive(button, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, TRUE, 2);

    /* set game object pointers */
    gskat.area = area;
    gskat.window = window;

    /* fill widgets hash table */
    g_hash_table_insert(gskat.widgets, "window", window);
    g_hash_table_insert(gskat.widgets, "button", button);
    g_hash_table_insert(gskat.widgets, "lb_stich", lb_game_stich_right);
    g_hash_table_insert(gskat.widgets, "lb_re", lb_game_re_right);
    g_hash_table_insert(gskat.widgets, "lb_spiel", lb_game_spiel_right);
    g_hash_table_insert(gskat.widgets, "lb_gereizt", lb_game_gereizt_right);
    g_hash_table_insert(gskat.widgets, "lb_rank_p1", lb_rank_p1);
    g_hash_table_insert(gskat.widgets, "lb_rank_p2", lb_rank_p2);
    g_hash_table_insert(gskat.widgets, "lb_rank_p3", lb_rank_p3);
    g_hash_table_insert(gskat.widgets, "frame_game", frame_game);
    g_hash_table_insert(gskat.widgets, "table_rank", table_rank);
    g_hash_table_insert(gskat.widgets, "lb_p1_name", lb_rank_p1_name);
    g_hash_table_insert(gskat.widgets, "lb_p2_name", lb_rank_p2_name);
    g_hash_table_insert(gskat.widgets, "lb_p3_name", lb_rank_p3_name);
    g_hash_table_insert(gskat.widgets, "statusbar", statusbar);
    g_hash_table_insert(gskat.widgets, "vtable", vtable);

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

    gtk_widget_add_events(area, GDK_BUTTON_PRESS_MASK |
            GDK_POINTER_MOTION_MASK);
    g_signal_connect(G_OBJECT(area), "button_press_event",
            G_CALLBACK(mouse_click), NULL);
    g_signal_connect(G_OBJECT(area), "motion-notify-event",
            G_CALLBACK(mouse_move), NULL);
}

/**
 * update_interface:
 *
 * Update all game interface elements
 */
void update_interface(void)
{
    const gint len = 1024;
    gchar *text;

    /* 1024 characters should be enough ... */
    text = (gchar *) g_malloc(sizeof(gchar) * len);

    /* current trick label */
    g_snprintf(text, len-1, "%d", gskat.stich);
    gtk_label_set_text(GTK_LABEL(get_widget("lb_stich")), text);

    /* re player label */
    g_snprintf(text, len-1, "%s", (gskat.re) ? gskat.re->name : "-");
    gtk_label_set_text(GTK_LABEL(get_widget("lb_re")), text);

    /* current game/trump label */
    g_snprintf(text, len-1, "%s", (gskat.trump != -1) ?
            suit_name(gskat.trump) : "-");
    gtk_label_set_text(GTK_LABEL(get_widget("lb_spiel")), text);

    /* bidden value */
    if (gskat.re)
        g_snprintf(text, len-1, "%d", gskat.re->gereizt);
    else
        g_snprintf(text, len-1, "-");
    gtk_label_set_text(GTK_LABEL(get_widget("lb_gereizt")), text);

    /* current round label */
    g_snprintf(text, len-1, _("Round %d"), gskat.round);
    gtk_frame_set_label(GTK_FRAME(get_widget("frame_game")), text);

    g_free(text);
}

/**
 * update_rank_interface:
 *
 * Update the players' points on the right-hand interface
 *
 * TODO: The table should only be extended if a game round was
 * properly played until the end.
 */
void update_rank_interface(void)
{
    gint i, len = 0;
    gchar msg[32];
    player *cur;
    GtkWidget *rank_label;
    GtkTable *table = GTK_TABLE(get_widget("table_rank"));

    /* update sum of points */
    g_sprintf(msg, "<b>%d</b>", gskat.players[0]->sum_points);
    gtk_label_set_markup(GTK_LABEL(get_widget("lb_rank_p1")), msg);
    g_sprintf(msg, "<b>%d</b>", gskat.players[1]->sum_points);
    gtk_label_set_markup(GTK_LABEL(get_widget("lb_rank_p2")), msg);
    g_sprintf(msg, "<b>%d</b>", gskat.players[2]->sum_points);
    gtk_label_set_markup(GTK_LABEL(get_widget("lb_rank_p3")), msg);

    /* get the number of rows */
    g_object_get(G_OBJECT(table), "n-rows", &len, NULL);

    /* add a new row in the table_rank list */
    gtk_table_resize(table, len+1, 3);

    for (i=0; i<3; ++i)
    {
        cur = gskat.players[i];

        /* get last entry in round_points list */
        g_sprintf(msg, "%d", GPOINTER_TO_INT(g_list_nth_data(cur->round_points,
                        g_list_length(cur->round_points)-1)));
        rank_label = gtk_label_new(msg);

        gtk_table_attach_defaults(table, rank_label, i, i+1, len, len+1);
    }

    gtk_widget_show_all(GTK_WIDGET(table));
}

/**
 * load_card:
 * @list:  pointer to a #GList to put the loaded card into
 * @file:  file to load the card image from
 * @rank:  card rank
 * @suit:  card suit
 *
 * Load the card image of the given rank and suit
 */
void load_card(GList **list, const gchar *file, gint rank, gint suit)
{
    card *tcard = (card *) g_malloc(sizeof(card));

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

/**
 * load_cards:
 * @paths:  several possible paths to search the card image files in
 *
 * Allocate and initialize all 32 game cards
 *
 * Returns: %TRUE on success, otherwise %FALSE
 */
gboolean load_cards(const gchar **paths)
{
    gint i, j, id, max = max_str_lenv(paths) + 30;
    gint ranks[] = { 1, 7, 8, 9, 10, 11, 12, 13 };
    const gchar *path;
    GList **list = &(gskat.cards);
    gboolean error = TRUE;

    gchar *cname = (gchar *) g_malloc(sizeof(gchar) * max);

    /* save all loaded card ids in here so we don't load the same card twice */
    gint *loaded = (gint *) g_malloc0(sizeof(gint) * 120);

    /* try all given paths if necessary */
    for (path = *paths; path; path = *paths++)
    {
        if (!g_file_test(path, G_FILE_TEST_EXISTS))
            continue;

        error = FALSE;

        for (i=0; i<4; ++i)
        {
            for (j=0; j<8; ++j)
            {
                id = SUITS[i] + ranks[j];

                /* skip already loaded cards */
                if (loaded[id])
                    continue;

                g_sprintf(cname, "%s/%d.png", path, id);

                if (g_file_test(cname, G_FILE_TEST_EXISTS))
                {
                    load_card(list, cname, ranks[j], SUITS[i]);
                    loaded[id] = 1;
                    gskat_msg(MT_INFO, _("Loading '%s' ... OK\n"), cname);
                }
                else
                {
                    error = TRUE;
                    gskat_msg(MT_ERROR, _("Failed to load '%s'\n"), cname);
                }
            }
        }

        /* load back of cards */
        if (!gskat.back)
        {
            g_sprintf(cname, "%s/back.png", path);
            gskat.back = load_image(cname);
        }

        /* load back of cards */
        if (!gskat.bg)
        {
            g_sprintf(cname, "%s/bg.png", path);
            gskat.bg = load_image(cname);
        }

        if (!error)
            break;
    }

    g_free(cname);
    g_free(loaded);

    return !error;
}

/**
 * load_image:
 * @filename:  filename of the card image to load
 *
 * Load an image and create a new #cairo_surface_t on its basis
 *
 * Returns: (transfer full): the new #cairo_surface_t object
 */
cairo_surface_t *load_image(gchar *filename)
{

    if (g_file_test(filename, G_FILE_TEST_EXISTS))
    {
        gskat_msg(MT_INFO, _("Loading '%s' ... OK\n"), filename);
        return cairo_image_surface_create_from_png(filename);
    }
    else
    {
        gskat_msg(MT_ERROR, _("Failed to load image '%s'\n"), filename);
        return NULL;
    }
}

/**
 * get_widget:
 * @name:  Key of the widget the return
 *
 * Returns the #GtkWidget with the unique key given by @name
 */
GtkWidget *get_widget(const gchar *name)
{
    GtkWidget *widget = g_hash_table_lookup(gskat.widgets, name);

    g_warn_if_fail(widget != NULL);

    return widget;
}

/**
 * free_app:
 *
 * Free all allocated in-game memory
 */
void free_app(void)
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
    g_strfreev(gskat.player_names);
    gskat.player_names = NULL;

    /* free played stiche */
    for (i=0; i<10; ++i)
    {
        if (gskat.stiche[i])
        {
            g_free(gskat.stiche[i]->cards);
            g_free(gskat.stiche[i]);

            gskat.stiche[i] = NULL;
        }
    }
    g_free(gskat.stiche);
    gskat.stiche = NULL;

    /* free icons */
    if (gskat.icons)
    {
        for (i=0; i<4; i++)
        {
            if (gskat.icons[i])
            {
                g_object_unref(gskat.icons[i]);
                gskat.icons[i] = NULL;
            }
        }
        g_free(gskat.icons);
    }

    /* free bugreport log */
    if (gskat.log)
        g_string_free(gskat.log, TRUE);
    gskat.log = NULL;

    /* free remaining objects */
    if (gskat.back)
        cairo_surface_destroy(gskat.back);
    gskat.back = NULL;
    if (gskat.bg)
        cairo_surface_destroy(gskat.bg);
    gskat.bg = NULL;

    gdk_cursor_unref(gskat.pirate_cursor);
    gdk_cursor_unref(gskat.hand_cursor);

    g_hash_table_destroy(gskat.config);
    g_hash_table_destroy(gskat.widgets);

    if (gskat.datadir)
        g_free(gskat.datadir);

    gskat.config = NULL;

    gskat_msg(MT_INFO, _("Quit gskat\n"));
}

/* vim:set et sw=4 sts=4 tw=80: */
