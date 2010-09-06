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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "def.h"

/**
 * @brief Enumeration of different configuration value types
 */
typedef enum
{
    INT,
    DOUBLE,
    BOOL,
    STR
} property_type;

/**
 * @brief Configuration value structure
 */
typedef struct
{
    property_type type;
    union
    {
        gint *i;
        gdouble *d;
        gboolean *b;
        gchar **s;
    } ptr;
} property_value;

/**
 * @brief Configuration element structure
 */
typedef struct
{
    const gchar *name;
    property_value pval;
} property;

void load_config();

gchar *get_config_dir();

void set_config_filename();

gboolean create_conf_dir(const gchar *config_dir);

void get_config_value(GKeyFile *keyfile, property *prop);

gboolean set_config_value(GKeyFile *keyfile, property *prop);

gboolean write_config();

gboolean read_config();

void set_default_config();

#endif /* __CONFIG_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
