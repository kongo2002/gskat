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
#include "config.h"

/**
 * @brief Load the gskat configuration file
 *
 * If the config file cannot be found the default configuration values
 * are written to a new config file.
 */
void load_config()
{
    gchar *filename, *config_dir = NULL;

    /* get XDG config directory */
    config_dir = (gchar *) g_get_user_config_dir();

    if (!config_dir)
    {
        /* get home directory */
        const gchar *home_dir = g_getenv("HOME");
        if (!home_dir)
            home_dir = g_get_home_dir();

        config_dir = (gchar *) home_dir;
    }

    if (!gskat.conf->filename)
    {
        filename = g_strconcat(config_dir, "/gskat/gskat.conf", NULL);
        gskat.conf->filename = filename;
    }
    else
        filename = gskat.conf->filename;

    /* try to find config file */
    if (filename && g_file_test(filename, G_FILE_TEST_EXISTS))
    {
        DPRINT(("Found config file '%s'\n", filename));

        read_config();
    }
    else
    {
        DPRINT(("Failed to load config from '%s'\n", filename));
        DPRINT(("Using default settings instead.\n"));

        /* set default values */
        set_default_config();

        /* try to save config */
        if (create_conf_dir(config_dir))
            write_config();
    }
}

/**
 * @brief Allocate the configuration structure and set the default values
 */
void alloc_config()
{
    gskat.conf = (config *) g_malloc(sizeof(config));

    if (gskat.conf)
    {
        gskat.conf->player_names = NULL;
        gskat.conf->gui = TRUE;
        gskat.conf->animation = TRUE;
        gskat.conf->anim_duration = 200;
        gskat.conf->show_tricks = TRUE;
        gskat.conf->num_show_tricks = 1;
        gskat.conf->debug = FALSE;
        gskat.conf->filename = NULL;
    }
}

/**
 * @brief Set the configuration to the default values
 */
void set_default_config()
{
    /* set player names */
    const gchar *user_name = g_getenv("USER");

    gskat.conf->player_names = (gchar **) g_malloc(sizeof(gchar *) * 4);

    gskat.conf->player_names[0] = g_strdup(user_name ? user_name : "Player");
    gskat.conf->player_names[1] = g_strdup("Cuyo");
    gskat.conf->player_names[2] = g_strdup("Dozo");
    gskat.conf->player_names[3] = NULL;
}

/**
 * @brief Write the current configuration to the config file
 */
gboolean write_config()
{
    gsize length;
    gchar *filename = gskat.conf->filename;
    gboolean done = FALSE;
    gchar *key_file_content = NULL;
    GKeyFile *keys = NULL;
    GError *err = NULL;

    keys = g_key_file_new();

    if (keys)
    {
        g_key_file_set_string_list(keys, "gskat", "player_names",
                (const gchar **) gskat.conf->player_names, 3);

        g_key_file_set_boolean(keys, "gskat", "gui", gskat.conf->gui);
        g_key_file_set_boolean(keys, "gskat", "animation",
                gskat.conf->animation);
        g_key_file_set_integer(keys, "gskat", "anim_duration",
                gskat.conf->anim_duration);
        g_key_file_set_boolean(keys, "gskat", "show_tricks",
                gskat.conf->show_tricks);
        g_key_file_set_integer(keys, "gskat", "num_show_tricks",
                gskat.conf->num_show_tricks);
        g_key_file_set_boolean(keys, "gskat", "debug", gskat.conf->debug);

        key_file_content = g_key_file_to_data(keys, &length, NULL);

        g_key_file_free(keys);
    }

    if (key_file_content)
    {
        done = g_file_set_contents(filename, key_file_content, length, &err);

        if (done)
            DPRINT(("Saved configuration: %s\n", filename));
        else
        {
            DPRINT(("Failed to save configuration: %s\n", filename));

            g_printerr("%s\n", err->message);
            g_clear_error(&err);
        }

        g_free(key_file_content);
    }

    return done;
}

/**
 * @brief Read the configuration values from the config file
 */
gboolean read_config()
{
    gsize length;
    gchar *filename = gskat.conf->filename;
    gboolean done = FALSE;
    GError *error = NULL;
    GKeyFile *keyfile = NULL;

    keyfile = g_key_file_new();

    if (keyfile)
    {
        done = g_key_file_load_from_file(keyfile, filename,
                G_KEY_FILE_NONE, &error);

        if (error)
        {
            DPRINT(("Failed to read configuration: %s\n", error->message));
            g_clear_error(&error);
        }

        /* TODO: this needs to be refactored/modularized */
        if (done)
        {
            /* read player names */
            gskat.conf->player_names = g_key_file_get_string_list(keyfile,
                    "gskat", "player_names", &length, &error);

            if (error)
            {
                DPRINT(("Failed to read player names.\n"));
                g_clear_error(&error);
            }

            /* read other values */
            gskat.conf->gui = g_key_file_get_boolean(keyfile, "gskat",
                    "gui", &error);

            if (error)
            {
                DPRINT(("Failed to read 'gui' value from config file.\n"));
                g_clear_error(&error);
            }

            gskat.conf->animation = g_key_file_get_boolean(keyfile, "gskat",
                    "animation", &error);

            if (error)
            {
                DPRINT(("Failed to read 'animation' value from config \
                            file.\n"));
                g_clear_error(&error);
            }

            gskat.conf->anim_duration = g_key_file_get_integer(keyfile, "gskat",
                    "anim_duration", &error);

            if (error)
            {
                DPRINT(("Failed to read 'anim_duration' value from config \
                            file.\n"));
                g_clear_error(&error);
            }

            gskat.conf->show_tricks = g_key_file_get_boolean(keyfile, "gskat",
                    "show_tricks", &error);

            if (error)
            {
                DPRINT(("Failed to read 'show_tricks' value from config \
                            file.\n"));
                g_clear_error(&error);
            }

            gskat.conf->num_show_tricks = g_key_file_get_integer(keyfile,
                    "gskat", "num_show_tricks", &error);

            if (error)
            {
                DPRINT(("Failed to read 'num_show_tricks' value from config \
                            file.\n"));
                g_clear_error(&error);
            }

            gskat.conf->debug = g_key_file_get_boolean(keyfile, "gskat",
                    "debug", &error);

            if (error)
            {
                DPRINT(("Failed to read 'debug' value from config file.\n"));
                g_clear_error(&error);
            }

        }

        g_key_file_free(keyfile);
    }

    return done;
}

/**
 * @brief Create the directory for the config file if necessary
 */
gboolean create_conf_dir(const gchar *config_dir)
{
    gboolean done = FALSE, exists = FALSE;
    gchar *gtk_dir = g_strconcat(config_dir, "/gskat", NULL);

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
