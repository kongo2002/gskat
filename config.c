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
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include "def.h"
#include "config.h"

void load_config(app *app)
{
    gchar *filename;

    /* get home directory */
    const gchar *home_dir = g_getenv("HOME");
    if (!home_dir)
        home_dir = g_get_home_dir();

    filename = g_strconcat(home_dir, "/.gskat/conf", NULL);

    app->player_names = (gchar **) g_malloc(sizeof(gchar *) * 3);

    /* try to find config file */
    if (filename && g_file_test(filename, G_FILE_TEST_EXISTS))
    {
        DPRINT(("Found config file '%s'\n", filename));

        /* read config
         * set player names */
    }
    /* set default values */
    else
    {
        const gchar *user_name = g_getenv("USER");

        DPRINT(("Failed to load config from '%s'\n", filename));
        DPRINT(("Using default settings instead.\n"));

        app->player_names[0] = g_strdup(user_name ? user_name : "Player");
        app->player_names[1] = g_strdup("Cuyo");
        app->player_names[2] = g_strdup("Dozo");
    }

    g_free(filename);
}

gboolean create_conf_dir(app *app, const gchar *home)
{
    gboolean done = FALSE, exists = FALSE;
    gchar *gtk_dir = g_strconcat(home, "/.gskat", NULL);

    if (gtk_dir)
    {
        exists = g_file_test(gtk_dir, G_FILE_TEST_EXISTS);

        if (!exists && g_mkdir(gtk_dir, 0755) != 0)
        {
            DPRINT(("Unable to create directory: %s\n", gtk_dir));
        }
        else
            done = TRUE;

        g_free(gtk_dir);
    }

    return done;
}

/* vim:set et sw=4 sts=4 tw=80: */
