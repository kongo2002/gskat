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

/* configuration initialization table */
struct {
    const gchar *name;
    property_type t;
    property_widget w;
} config_values[] = {
    { "animation"         , BOOL , TOGGLEBUTTON },
    { "show_tricks"       , BOOL , TOGGLEBUTTON },
    { "num_show_tricks"   , INT  , SPINBUTTON },
    { "anim_duration"     , INT  , SPINBUTTON },
    { "reaction"          , BOOL , TOGGLEBUTTON },
    { "reaction_duration" , INT  , SPINBUTTON },
    { "show_poss_cards"   , BOOL , TOGGLEBUTTON },
    { "debug"             , BOOL , TOGGLEBUTTON },
    { NULL                , 0    , 0 }
};

/**
 * new_property:
 * @name:   name of the property
 * @type:   #property_type of the property
 * @widget: #property_widget type of the property
 *
 * Create a new #property object
 *
 * Returns: a new #property object
 */
property *new_property(const gchar *name, property_type type,
        property_widget widget)
{
    property *p = (property *) g_malloc0(sizeof(property));

    p->name = g_strdup(name);
    p->pval.type = type;
    p->pval.wtype = widget;
    p->widget = NULL;

    return p;
}

/**
 * init_config:
 *
 * Initialize the configuration hash table
 */
void init_config(void)
{
    gint i;

    /* TODO: probably we want to define a better
     * value_destroy_func than g_free() */
    gskat.config = g_hash_table_new_full(g_str_hash, g_str_equal,
            g_free, g_free);

    for (i=0; config_values[i].name; ++i)
    {
        g_hash_table_insert(gskat.config, (gpointer) config_values[i].name,
                new_property(config_values[i].name, config_values[i].t,
                    config_values[i].w));
    }
}

/**
 * get_prop:
 * @name:  Name of the property
 *
 * Search for the given @name in the configuration hash table.
 *
 * Returns: a #gpointer to the property value or %NULL
 */
gpointer get_prop(const gchar *name)
{
    property *p = g_hash_table_lookup(gskat.config, name);

    g_return_val_if_fail(p, NULL);

    switch (p->pval.type)
    {
        case INT:
            return p->pval.ptr.i;
        case BOOL:
            return p->pval.ptr.b;
        case DOUBLE:
            return p->pval.ptr.d;
        case STR:
            return p->pval.ptr.s;
        case STRV:
            return p->pval.ptr.v;
        default:
            return NULL;
    }
}

/**
 * get_prop_strv:
 * @name:   Name of the property
 * @index:  Index of the value to return
 *
 * Search for the value at @index inside a string array property
 *
 * Returns: the string at the given index or %NULL
 */
gchar *get_prop_strv(const gchar *name, gint index)
{
    gint i;
    property *p = g_hash_table_lookup(gskat.config, name);

    g_return_val_if_fail(p, NULL);
    g_return_val_if_fail(p->pval.type == STRV, NULL);

    /* check for out of bounds array index */
    for (i=0; i<=index; ++i)
    {
        if (p->pval.ptr.v[i] == NULL)
            return NULL;
    }

    return p->pval.ptr.v[index];
}

/**
 * set_prop:
 * @name:   Name of the property
 * @value:  #gpointer to the new value
 *
 * Set a new value of the given property.
 */
void set_prop(const gchar *name, gpointer value)
{
    property *p = g_hash_table_lookup(gskat.config, name);
    g_return_if_fail(p);

    switch (p->pval.type)
    {
        case INT:
            p->pval.ptr.i = value;
            break;
        case BOOL:
            p->pval.ptr.b = value;
            break;
        case DOUBLE:
            p->pval.ptr.d = value;
            break;
        case STR:
            p->pval.ptr.s = value;
            break;
        case STRV:
            p->pval.ptr.v = value;
            break;
    }
}

/**
 * set_prop_widget:
 * @name:    Name of the property
 * @widget:  #GtkWidget to be associated with the property
 *
 * Set the #GtkWidget of the given property.
 */
void set_prop_widget(const gchar *name, GtkWidget *widget)
{
    property *p = g_hash_table_lookup(gskat.config, name);
    g_return_if_fail(p);

    p->widget = widget;
}

/**
 * get_prop_widget_val:
 * @key:   Key (name) of the property
 * @val:   #property object
 * @data:  arbitrary user data
 *
 * Set the property to the value of the associated #GtkWidget.
 * This function is used with g_hash_table_foreach()
 */
void get_prop_widget_val(gpointer key, gpointer val, gpointer data)
{
    UNUSED(key);
    UNUSED(data);
    const gchar *entry_val;
    property *prop = (property *) val;

    g_return_if_fail(prop);
    g_return_if_fail(prop->widget);

    switch (prop->pval.wtype)
    {
        case SPINBUTTON:
            *prop->pval.ptr.i = gtk_spin_button_get_value_as_int(
                    GTK_SPIN_BUTTON(prop->widget));
            break;
        case TOGGLEBUTTON:
            *prop->pval.ptr.b = gtk_toggle_button_get_active(
                    GTK_TOGGLE_BUTTON(prop->widget));
            break;
        case ENTRY:
            entry_val = gtk_entry_get_text(GTK_ENTRY(prop->widget));

            if (prop->pval.ptr.s)
            {
                if (strcmp(prop->pval.ptr.s, entry_val))
                    g_free(prop->pval.ptr.s);
                else
                    break;
            }
            prop->pval.ptr.s = g_strdup(entry_val);
            break;
    }

    /* reset widget pointer */
    prop->widget = NULL;
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
    gchar *filename = g_build_filename(get_config_dir(), "gskat.conf", NULL);

    /* try to find config file */
    if (filename && g_file_test(filename, G_FILE_TEST_EXISTS))
    {
        gskat_msg(MT_INFO, _("Found config file '%s'\n"), filename);

        read_config(filename);
    }
    else
    {
        gskat_msg(MT_ERROR, _("Failed to load config from '%s'\n"), filename);
        gskat_msg(MT_ERROR, _("Using default settings instead.\n"));

        /* try to save config */
        write_config(filename);
    }

    g_free(filename);
}

/**
 * set_bool_val:
 * @name:  Name of the property
 * @val:   #gboolean value
 *
 * Set a boolean property value
 */
void set_bool_val(const gchar *name, gboolean val)
{
    gboolean **ptr;
    property *p = (property *) g_hash_table_lookup(gskat.config, name);

    g_return_if_fail(p);

    ptr = &p->pval.ptr.b;

    if (!*ptr)
        *ptr = (gboolean *) g_malloc(sizeof(gboolean));

    **ptr = val;
}

/**
 * set_int_val:
 * @name:  Name of the property
 * @val:   #gint value
 *
 * Set an integer property value
 */
void set_int_val(const gchar *name, gint val)
{
    gint **ptr;
    property *p = (property *) g_hash_table_lookup(gskat.config, name);

    g_return_if_fail(p);

    ptr = &p->pval.ptr.i;

    if (!*ptr)
        *ptr = (gint *) g_malloc(sizeof(gint));

    **ptr = val;
}

/**
 * set_default_config:
 *
 * Set the configuration to the default values
 */
void set_default_config(void)
{
    const gchar *user_name = g_getenv("USER");

    set_bool_val("animation", TRUE);
    set_int_val("anim_duration", 200);
    set_bool_val("reaction", TRUE);
    set_int_val("reaction_duration", 500);
    set_bool_val("show_tricks", TRUE);
    set_int_val("num_show_tricks", 1);
    set_bool_val("show_poss_cards", TRUE);
    set_bool_val("debug", FALSE);

    /* set player names */
    gskat.player_names = (gchar **) g_malloc(sizeof(gchar *) * 4);

    gskat.player_names[0] = g_strdup(user_name ? user_name : "Player");
    gskat.player_names[1] = g_strdup("Cuyo");
    gskat.player_names[2] = g_strdup("Dozo");
    gskat.player_names[3] = NULL;
}

/**
 * write_config:
 * @filename:  Filename of the configuration file
 *
 * Write the current configuration to the config file
 *
 * Returns: %TRUE on success, otherwise %FALSE
 */
gboolean write_config(const gchar *filename)
{
    gsize length;
    gboolean done = FALSE;
    gchar *key_file_content = NULL;
    GKeyFile *keys = NULL;
    GError *err = NULL;

    keys = g_key_file_new();

    /* write player names into keyfile */
    g_key_file_set_string_list(keys, "gskat", "player_names",
            (const gchar **) gskat.player_names, 3);

    /* add all remaining config values to keyfile content */
    g_hash_table_foreach(gskat.config, get_config_value, keys);

    key_file_content = g_key_file_to_data(keys, &length, NULL);

    g_key_file_free(keys);

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
 * @key:   Key (name) of the property
 * @val:   #property object
 * @data:  #GKeyFile
 *
 * Write a configuration value into the given keyfile in order to
 * write a new config file to disk.
 * This function is used with g_hash_table_foreach()
 */
void get_config_value(gpointer key, gpointer val, gpointer data)
{
    UNUSED(key);
    gint i;
    property *prop = (property *) val;
    GKeyFile *keyfile = (GKeyFile *) data;

    g_return_if_fail(prop);
    g_return_if_fail(keyfile);

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
                    prop->pval.ptr.s);
            break;
        case STRV:
            /* get string array length */
            for (i=0; prop->pval.ptr.v[i]; ++i);

            g_key_file_set_string_list(keyfile, "gskat", prop->name,
                    (const gchar **) prop->pval.ptr.v, i-1);
            break;
    }
}
/**
 * set_config_value:
 * @key:   Key (name) of the property
 * @val:   #property object
 * @data:  #GKeyFile
 *
 * Read a configuration value from the given #GKeyFile and apply
 * the new value to the property inside the configuration hash table.
 * This function is used with g_hash_table_foreach()
 */
void set_config_value(gpointer key, gpointer val, gpointer data)
{
    UNUSED(key);
    property *prop = (property *) val;
    gsize length;
    GKeyFile *keyfile = (GKeyFile *) data;
    GError *error = NULL;

    g_return_if_fail(prop);
    g_return_if_fail(keyfile);

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
            if (prop->pval.ptr.s)
                g_free(prop->pval.ptr.s);

            prop->pval.ptr.s = g_key_file_get_string(keyfile, "gskat",
                    prop->name, &error);
            break;
        case STRV:
            if (prop->pval.ptr.v)
                g_strfreev(prop->pval.ptr.v);

            prop->pval.ptr.v = g_key_file_get_string_list(keyfile, "gskat",
                    prop->name, &length, &error);
    }

    if (error)
    {
        gskat_msg(MT_ERROR,
                _("Failed to read '%s' from config file.\n"), prop->name);
        g_clear_error(&error);
    }
}

/**
 * read_config:
 * @filename:  Filename of the configuration file
 *
 * Read the configuration values from the config file
 *
 * Returns: %TRUE on success, otherwise %FALSE
 */
gboolean read_config(const gchar *filename)
{
    gsize length;
    gchar **names = NULL;
    gboolean done = FALSE, failed = FALSE;
    GError *error = NULL;
    GKeyFile *keyfile = NULL;

    keyfile = g_key_file_new();

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
        g_strfreev(gskat.player_names);
        gskat.player_names = names;
    }

    /* read all remaining config values */
    g_hash_table_foreach(gskat.config, set_config_value, keyfile);

    /* rewrite config if not all values could be read successfully */
    if (!done || failed)
    {
        gskat_msg(MT_INFO, _("Rewriting config file.\n"));
        write_config(filename);
    }

    g_key_file_free(keyfile);

    return done;
}

/* vim:set et sw=4 sts=4 tw=80: */
