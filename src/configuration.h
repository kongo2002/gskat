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
 * property_type:
 * @INT:    #gint type
 * @DOUBLE: #gdouble type
 * @BOOL:   #gboolean type
 * @STR:    #gchar type
 *
 * Enumeration of different configuration value types
 */
typedef enum {
    INT,
    DOUBLE,
    BOOL,
    STR
} property_type;

/**
 * property_value:
 * @type: #property_type structure
 *
 * Configuration value structure
 */
typedef struct _property_value {
    property_type type;
    union {
        gint *i;
        gdouble *d;
        gboolean *b;
        gchar **s;
    } ptr;
} property_value;

/**
 * property:
 * @name: Name of the property
 * @pval: #property_value structure containing type and pointer of the property
 *
 * Configuration element structure
 */
typedef struct _property {
    const gchar *name;
    property_value pval;
} property;

void load_config(void);

void set_config_filename(void);

void get_config_value(GKeyFile *keyfile, property *prop);

gboolean set_config_value(GKeyFile *keyfile, property *prop);

gboolean write_config(void);

gboolean read_config(void);

void set_default_config(void);

#endif /* __CONFIG_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
