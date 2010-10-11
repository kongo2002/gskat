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
#include "interface.h"

/**
 * swap:
 * @a:  first integer
 * @b:  second integer
 *
 * Helper function to swap two integers
 */
void swap(gint *a, gint *b)
{
    gint tmp = *a;
    *a = *b;
    *b = tmp;
}

/**
 * max_str_l:
 * @first:  first string to check
 * @args:   additional strings to check
 * @array:  %NULL-terminated string array
 *
 * Return maximum string length
 *
 * Returns: maximum string length of all given strings
 */
static gint max_str_l(const gchar *first, va_list *args, const gchar **array)
{
    gint len, max = -1;
    const gchar *str;

    if (array)
        str = *array;
    else
        str = first;

    while (str)
    {
        len = strlen(str);

        if (max == -1 || len > max)
            max = len;

        if (array)
            str = *array++;
        else
            str = va_arg(*args, gchar *);
    }

    return max;
}

/**
 * max_str_len:
 * @first:  first string to check
 * @...:    additional strings to check
 *
 * Return maximum string length
 *
 * Returns: maximum string length of all given strings
 */
gint max_str_len(const gchar *first, ...)
{
    gint max;
    va_list args;

    va_start(args, first);
    max = max_str_l(first, &args, NULL);
    va_end(args);

    return max;
}

/**
 * max_str_lenv:
 * @args:  %NULL-terminated string array
 *
 * Return maximum string length
 *
 * Returns: maximum string length of all given strings
 */
gint max_str_lenv(const gchar **args)
{
    return max_str_l(NULL, NULL, args);
}

/**
 * write_to_log:
 * @fmt:   printf-like format string
 * @args:  optional arguments
 *
 * Write the given string into the bugreport log string
 */
void write_to_log(const gchar *fmt, va_list args)
{
    gchar *str;

    if (!gskat.log)
    {
        str = g_strdup_vprintf(fmt, args);
        gskat.log = g_string_new(str);

        g_free(str);
        return;
    }

    g_string_append_vprintf(gskat.log, fmt, args);
}

/**
 * gskat_msg:
 * @type: #msg_type defining the type of the message
 * @fmt:  printf-like format string
 * @...:  optional arguments
 *
 * Print or log a message for debug, information or error purpose.
 * Furthermore the @type defines if the message is shown in the statusbar
 * or shown in a message dialog window and if the message is stored inside
 * the bugreport log.
 *
 * If the MT_GAME @msg_type is set then the message is printed or shown
 * regardless of the current log level.
 */
void gskat_msg(msg_type type, const gchar *fmt, ...)
{
    va_list args;
    msg_type level = (type & MT_LEVEL_MASK);

#ifndef DEBUG
    if (fmt && ((type & MT_GAME) || (level <= gskat.log_level)))
#else
    if (fmt)
#endif
    {
        /* check if message should be logged for the bug report */
        if (type & MT_BUGREPORT)
        {
            va_start(args, fmt);

            write_to_log(fmt, args);

            va_end(args);
        }

        /* check if the message should be shown in the statusbar */
        if (type & MT_STATUSBAR)
        {
            va_start(args, fmt);

            update_sb(fmt, args);

            va_end(args);
        }

        /* check if a message dialog window should be shown */
        if (type & MT_DIALOG)
        {
            va_start(args, fmt);

            if (level <= MT_WARN)
                show_dialog_error(fmt, args);
            else
                show_dialog_info(fmt, args);

            va_end(args);
        }

        /* print according to current log level */
        if (level || (type & MT_GAME))
        {
            va_start(args, fmt);

            if (level <= MT_WARN)
                g_vfprintf(stderr, fmt, args);
            else
                g_vfprintf(stdout, fmt, args);

            va_end(args);
        }
    }
}

/**
 * get_config_dir:
 *
 * Determine the configuration directory
 *
 * First try to get the configuration directory according to
 * the XDG base directory specification. If that fails use the
 * home directory.
 *
 * Returns: (transfer none): configuration directory string (may not be freed)
 */
gchar *get_config_dir(void)
{
    const gchar *base_dir;
    static gchar *config_dir = NULL;

    if (!config_dir)
    {
        /* get XDG config directory */
        if ((base_dir = g_get_user_config_dir()) && *base_dir != '\0')
            config_dir = g_build_filename(base_dir, "gskat", NULL);
        else
        {
            /* get home directory */
            if (!(base_dir = g_getenv("HOME")))
                base_dir = g_get_home_dir();

            config_dir = g_build_filename(base_dir, ".gskat", NULL);
        }

        /* if the directory does not exist try to create it
         * otherwise return an empty string */
        if (!create_dir(config_dir))
            config_dir = "";
    }

    return config_dir;
}

/**
 * get_data_dir:
 *
 * Determine the data directory
 *
 * First try to get the data directory according to
 * the XDG base directory specification. If that fails use the
 * home directory.
 *
 * Returns: (transfer none): data directory string (may not be freed)
 */
gchar *get_data_dir(void)
{
    const gchar *base_dir;
    static gchar *data_dir = NULL;

    if (!data_dir)
    {
        /* get XDG data directory */
        if ((base_dir = g_get_user_data_dir()) && *base_dir != '\0')
            data_dir = g_build_filename(base_dir, "gskat", NULL);
        else
        {
            /* get home directory */
            if (!(base_dir = g_getenv("HOME")))
                base_dir = g_get_home_dir();

            data_dir = g_build_filename(base_dir, ".gskat", NULL);
        }

        /* if the directory does not exist try to create it
         * otherwise return an empty string */
        if (!create_dir(data_dir))
            data_dir = "";
    }

    return data_dir;
}

/**
 * get_cache_dir:
 *
 * Determine the cache directory
 *
 * First try to get the cache directory according to
 * the XDG base directory specification. If that fails use the
 * home directory.
 *
 * Returns: (transfer none): cache directory string (may not be freed)
 */
gchar *get_cache_dir(void)
{
    const gchar *base_dir;
    static gchar *cache_dir = NULL;

    if (!cache_dir)
    {
        /* get XDG cache directory */
        if ((base_dir = g_get_user_cache_dir()) && *base_dir != '\0')
            cache_dir = g_build_filename(base_dir, "gskat", NULL);
        else
        {
            /* get home directory */
            if (!(base_dir = g_getenv("HOME")))
                base_dir = g_get_home_dir();

            cache_dir = g_build_filename(base_dir, ".gskat", NULL);
        }

        /* if the directory does not exist try to create it
         * otherwise return an empty string */
        if (!create_dir(cache_dir))
            cache_dir = "";
    }

    return cache_dir;
}

/**
 * get_desktop_dir:
 *
 * Determine the desktop directory
 *
 * First try to get the desktop directory according to
 * the XDG base directory specification. If that fails use the
 * home directory.
 *
 * Returns: (transfer none): desktop directory string (may not be freed)
 */
gchar *get_desktop_dir(void)
{
    const gchar *base_dir;
    static gchar *desktop_dir = NULL;

    if (!desktop_dir)
    {
        /* get XDG cache directory */
        if ((base_dir = g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP))
                && *base_dir != '\0')
            desktop_dir = g_build_filename(base_dir, "gskat", NULL);
        else
        {
            /* get home directory */
            if (!(base_dir = g_getenv("HOME")))
                base_dir = g_get_home_dir();

            desktop_dir = g_build_filename(base_dir, ".gskat", NULL);
        }

        /* if the directory does not exist try to create it
         * otherwise return an empty string */
        if (!create_dir(desktop_dir))
            desktop_dir = "";
    }

    return desktop_dir;
}

/**
 * create_dir:
 * @dir:  directory string
 *
 * Create the given directory if necessary
 *
 * Returns: %TRUE on success, otherwise %FALSE
 */
gboolean create_dir(const gchar *dir)
{
    gboolean exists;

    exists = g_file_test(dir, G_FILE_TEST_EXISTS);

    if (!exists && g_mkdir(dir, 0755) != 0)
    {
        gskat_msg(MT_DEBUG | MT_BUGREPORT,
                _("Unable to create directory: %s\n"), dir);
        return FALSE;
    }

    return TRUE;
}

/**
 * save_bugreport_to_file:
 * @filename:    File to save the bug report to
 * @time:        Current timestamp
 * @text_buffer: #GtkTextBuffer of the description text view
 *
 * Write the bug report string into the given filename
 */
void save_bugreport_to_file(const gchar *filename, GTimeVal *time,
        GtkTextBuffer *text_buffer)
{
    FILE *output;
    gchar *desc_text;

    if ((output = g_fopen(filename, "w")))
    {
        g_object_get(G_OBJECT(text_buffer), "text", &desc_text, NULL);

        g_fprintf(output, "gskat Version %s - %s\n", VERSION,
                GSKAT_COMMIT_HASH);
        g_fprintf(output, "%s\n", g_time_val_to_iso8601(time));
        g_fprintf(output, "------------------------------------------\n");
        g_fprintf(output, "%s", gskat.log->str);
        g_fprintf(output, "------------------------------------------\n");
        g_fprintf(output, "%s\n", desc_text);

        gskat_msg(MT_INFO | MT_STATUSBAR,
                _("Saved bug report to '%s'\n"), filename);

        fclose(output);
        g_free(desc_text);
    }
    else
        gskat_msg(MT_ERROR, _("Failed to save bug report to '%s'\n"), filename);
}

/* vim:set et sw=4 sts=4 tw=80: */
