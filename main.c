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

#include <gtk/gtk.h>
#include "def.h"
#include "interface.h"
#include "config.h"

static gboolean debug = FALSE;
static gboolean cli_mode = FALSE;

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
    { NULL }
};

int main(int argc, const char *argv[])
{
    GOptionContext *context;
    GError *error = NULL;

    /* initialization of game objects */
    app gskat =
    {
        /* lists */
        NULL, NULL, NULL, NULL, NULL, NULL,
        /* pixmaps and widgets */
        NULL, NULL, NULL, NULL, NULL,
        /* game state */
        LOADING,
        /* re player */
        NULL,
        /* integers */
        2, -1 , -1, 1, 1,
        /* hand/null game */
        FALSE, FALSE,
        /* configuration */
        NULL
    };

    /* parse command line arguments */
    context = g_option_context_new(" - GTK Skat");

    if (context)
    {
        g_option_context_add_main_entries(context, arguments, NULL);
        g_option_context_add_group(context, gtk_get_option_group(TRUE));

        if (!g_option_context_parse(context, &argc, (gchar ***) &argv, &error))
        {
            g_printerr("failed to parse arguments: %s\n", error->message);
            return 1;
        }

        g_option_context_free(context);

        /* allocate configuration */
        alloc_config(&gskat);

        /* load configuration */
        load_config(&gskat);

        /* toggle gui if desired */
        if (cli_mode)
            gskat.conf->gui = FALSE;

        /* initialize interface */
        create_interface(&gskat);

        /* show all widgets after being initialized */
        if (gskat.allwidgets != NULL)
        {
            gtk_widget_show_all(gskat.allwidgets[0]);
            gtk_main();

            free_app(&gskat);
        }
    }

    return 0;
}

/* vim:set et sw=4 sts=4 tw=80: */
