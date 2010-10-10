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

#ifndef __COMMON_H__
#define __COMMON_H__

#include "def.h"

void swap(gint *a, gint *b);

gint max_str_len(const gchar *first, ...);

gint max_str_lenv(const gchar **args);

void write_to_log(const gchar *fmt, va_list args);

void gskat_msg(msg_type type, const gchar *fmt, ...);

gchar *get_config_dir(void);

gchar *get_data_dir(void);

gchar *get_cache_dir(void);

gchar *get_desktop_dir(void);

gboolean create_dir(const gchar *dir);

void save_bugreport_to_file(const gchar *filename, GTimeVal *time,
        GtkTextBuffer *text_buffer);

#endif /* __COMMON_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
