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
#include "config.h"

app gskat;

static gboolean debug = FALSE;
static gboolean cli_mode = FALSE;
static gboolean no_animation = FALSE;

static GOptionEntry arguments[] =
{
    {
        "debug", 0, 0, G_OPTION_ARG_NONE, &debug,
        "toggle debug mode (for developing)", NULL
    },
    {
        "cli_mode", 0, 0, G_OPTION_ARG_NONE, &cli_mode,
        "disable GUI (command line only)", NULL
    },
    {
        "no_animation", 0, 0, G_OPTION_ARG_NONE, &no_animation,
        "disable card animations", NULL
    },
    { NULL, 0, 0, 0, NULL, NULL, NULL }
};

void initialize()
{
    gskat.cards      = NULL;
    gskat.skat       = NULL;
    gskat.table      = NULL;
    gskat.players    = NULL;
    gskat.stiche     = NULL;
    gskat.players    = NULL;
    gskat.icons      = NULL;
    gskat.back       = NULL;
    gskat.bg         = NULL;
    gskat.area       = NULL;
    gskat.allwidgets = NULL;
    gskat.state      = LOADING;
    gskat.re         = NULL;
    gskat.forehand   = 2;
    gskat.cplayer    = -1;
    gskat.trump      = -1;
    gskat.round      = 1;
    gskat.stich      = 1;
    gskat.hand       = FALSE;
    gskat.null       = FALSE;
    gskat.conf       = NULL;
}

int main(int argc, const char *argv[])
{
    GOptionContext *context;
    GError *error = NULL;

    /* initialization of game objects */
    initialize();

    /* parse command line arguments */
    context = g_option_context_new(" - GTK Skat");

    if (context)
    {
        g_option_context_add_main_entries(context, arguments, NULL);
        g_option_context_add_group(context, gtk_get_option_group(TRUE));

        if (!g_option_context_parse(context, &argc, (gchar ***) &argv, &error))
        {
            g_printerr("failed to parse arguments: %s\n", error->message);
            g_clear_error(&error);

            return 1;
        }

        g_option_context_free(context);

        /* allocate configuration */
        alloc_config();

        /* load configuration */
        load_config();

        /* toggle gui if desired */
        if (cli_mode)
            gskat.conf->gui = FALSE;

        if (no_animation)
            gskat.conf->animation = FALSE;

        /* initialize interface */
        create_interface();

        /* show all widgets after being initialized */
        if (gskat.allwidgets != NULL)
        {
            gtk_widget_show_all(gskat.allwidgets[0]);
            gtk_main();

            free_app();
        }
    }

    return 0;
}

/* vim:set et sw=4 sts=4 tw=80: */
