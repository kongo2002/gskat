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
#include "common.h"
#include "configuration.h"

/* convenience functions */
#ifndef PROP_MACRO
#define PROP_MACRO

#define INT_PROP(address)    { .type = INT   , .ptr.i = &(address) }
#define DOUBLE_PROP(address) { .type = DOUBLE, .ptr.d = &(address) }
#define BOOL_PROP(address)   { .type = BOOL  , .ptr.b = &(address) }
#define STR_PROP(address)    { .type = STR   , .ptr.s = &(address) }

#endif

/* configuration value table */
static const property config_values[] = {
    { "animation"         , BOOL_PROP(gskat.conf.animation) },
    { "show_tricks"       , BOOL_PROP(gskat.conf.show_tricks) },
    { "num_show_tricks"   , INT_PROP(gskat.conf.num_show_tricks) },
    { "anim_duration"     , INT_PROP(gskat.conf.anim_duration) },
    { "reaction"          , BOOL_PROP(gskat.conf.reaction) },
    { "reaction_duration" , INT_PROP(gskat.conf.reaction_duration) },
    { "show_poss_cards"   , BOOL_PROP(gskat.conf.show_poss_cards) },
    { "debug"             , BOOL_PROP(gskat.conf.debug) },
    { NULL                , { .type = INT, .ptr.i = NULL } }
};

/**
 * set_config_filename:
 *
 * Set the config filename string inside the global
 * configuration structure
 */
void set_config_filename(void)
{
    const gchar *config_dir = get_config_dir();

    gskat.conf.filename = g_build_filename(config_dir, "gskat.conf", NULL);
}

/**
 * load_config:
 *
 * Load the gskat configuration file
 *
 * If the config file cannot be found the default configuration values
 * are written to a new config file.
 */
void load_config(void)
{
    const gchar *filename;

    if (!gskat.conf.filename)
        set_config_filename();

    filename = gskat.conf.filename;

    /* try to find config file */
    if (filename && g_file_test(filename, G_FILE_TEST_EXISTS))
    {
        gskat_msg(MT_INFO, _("Found config file '%s'\n"), filename);

        read_config();
    }
    else
    {
        gskat_msg(MT_ERROR, _("Failed to load config from '%s'\n"), filename);
        gskat_msg(MT_ERROR, _("Using default settings instead.\n"));

        /* try to save config */
        write_config();
    }
}

/**
 * set_default_config:
 *
 * Set the configuration to the default values
 */
void set_default_config(void)
{
    const gchar *user_name = g_getenv("USER");

    gskat.conf.animation = TRUE;
    gskat.conf.anim_duration = 200;
    gskat.conf.reaction = TRUE;
    gskat.conf.reaction_duration = 500;
    gskat.conf.show_tricks = TRUE;
    gskat.conf.num_show_tricks = 1;
    gskat.conf.show_poss_cards = TRUE;
    gskat.conf.debug = FALSE;
    gskat.conf.filename = NULL;

    /* set player names */
    gskat.conf.player_names = (gchar **) g_malloc(sizeof(gchar *) * 4);

    gskat.conf.player_names[0] = g_strdup(user_name ? user_name : "Player");
    gskat.conf.player_names[1] = g_strdup("Cuyo");
    gskat.conf.player_names[2] = g_strdup("Dozo");
    gskat.conf.player_names[3] = NULL;
}

/**
 * write_config:
 *
 * Write the current configuration to the config file
 *
 * Returns: %TRUE on success, otherwise %FALSE
 */
gboolean write_config(void)
{
    gint i;
    gsize length;
    gchar *filename = gskat.conf.filename;
    gboolean done = FALSE;
    gchar *key_file_content = NULL;
    GKeyFile *keys = NULL;
    GError *err = NULL;

    keys = g_key_file_new();

    if (keys)
    {
        /* write player names into keyfile */
        g_key_file_set_string_list(keys, "gskat", "player_names",
                (const gchar **) gskat.conf.player_names, 3);

        /* add all remaining config values to keyfile content */
        for (i=0; config_values[i].name != NULL; ++i)
            get_config_value(keys, (property *) &config_values[i]);

        key_file_content = g_key_file_to_data(keys, &length, NULL);

        g_key_file_free(keys);
    }

    if (key_file_content)
    {
        done = g_file_set_contents(filename, key_file_content, length, &err);

        if (done)
            gskat_msg(MT_INFO, _("Saved configuration: %s\n"), filename);
        else
        {
            gskat_msg(MT_ERROR,
                    _("Failed to save configuration: %s\n"), filename);

            g_printerr("%s\n", err->message);
            g_clear_error(&err);
        }

        g_free(key_file_content);
    }

    return done;
}

/**
 * get_config_value:
 * @keyfile:  #GKeyFile containing the new values
 * @prop:     property element of the config_values array
 *
 * Write a configuration value in a keyfile in order to
 * write a new config file to disk
 */
void get_config_value(GKeyFile *keyfile, property *prop)
{
    if (!prop)
        return;

    switch (prop->pval.type)
    {
        case INT:
            g_key_file_set_integer(keyfile, "gskat", prop->name,
                    *prop->pval.ptr.i);
            break;
        case BOOL:
            g_key_file_set_boolean(keyfile, "gskat", prop->name,
                    *prop->pval.ptr.b);
            break;
        case DOUBLE:
            g_key_file_set_double(keyfile, "gskat", prop->name,
                    *prop->pval.ptr.d);
            break;
        case STR:
            g_key_file_set_string(keyfile, "gskat", prop->name,
                    *prop->pval.ptr.s);
            break;
        default:
            break;
    }
}
/**
 * set_config_value:
 * @keyfile:  #GKeyFile containing the parsed values
 * @prop:     property element of the config_values array
 *
 * Read a configuration value from @keyfile and apply
 * the new value to the global config structure
 *
 * Returns: %TRUE on success, otherwise %FALSE
 */
gboolean set_config_value(GKeyFile *keyfile, property *prop)
{
    GError *error = NULL;

    if (!prop)
        return FALSE;

    switch (prop->pval.type)
    {
        case INT:
            *prop->pval.ptr.i = g_key_file_get_integer(keyfile, "gskat",
                    prop->name, &error);
            break;
        case BOOL:
            *prop->pval.ptr.b = g_key_file_get_boolean(keyfile, "gskat",
                    prop->name, &error);
            break;
        case DOUBLE:
            *prop->pval.ptr.d = g_key_file_get_double(keyfile, "gskat",
                    prop->name, &error);
            break;
        case STR:
            *prop->pval.ptr.s = g_key_file_get_string(keyfile, "gskat",
                    prop->name, &error);
            break;
        default:
            return FALSE;

    }

    if (error)
    {
        gskat_msg(MT_ERROR,
                _("Failed to read '%s' from config file.\n"), prop->name);
        g_clear_error(&error);

        return FALSE;
    }

    return TRUE;
}

/**
 * read_config:
 *
 * Read the configuration values from the config file
 *
 * Returns: %TRUE on success, otherwise %FALSE
 */
gboolean read_config(void)
{
    gint i;
    gsize length;
    gchar **names = NULL;
    gchar *filename = gskat.conf.filename;
    gboolean done = FALSE, failed = FALSE;
    GError *error = NULL;
    GKeyFile *keyfile = NULL;

    keyfile = g_key_file_new();

    if (keyfile)
    {
        done = g_key_file_load_from_file(keyfile, filename,
                G_KEY_FILE_NONE, &error);

        if (error)
        {
            gskat_msg(MT_ERROR,
                    _("Failed to read configuration: %s\n"), error->message);
            g_clear_error(&error);

            return FALSE;
        }

        /* read player names */
        names = g_key_file_get_string_list(keyfile, "gskat", "player_names",
                &length, &error);

        if (error)
        {
            gskat_msg(MT_ERROR,
                    _("Failed to read 'player_names' from config file.\n"));
            failed = TRUE;
            g_clear_error(&error);
        }
        else
        {
            g_strfreev(gskat.conf.player_names);
            gskat.conf.player_names = names;
        }

        /* read all remaining config values */
        for (i=0; config_values[i].name != NULL; ++i)
        {
            if (!set_config_value(keyfile, (property *) &config_values[i]))
                failed = TRUE;
        }

        /* rewrite config if not all values could be read successfully */
        if (!done || failed)
        {
            gskat_msg(MT_INFO, _("Rewriting config file.\n"));
            write_config();
        }

        g_key_file_free(keyfile);
    }

    return done;
}

/* vim:set et sw=4 sts=4 tw=80: */
