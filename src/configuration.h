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

G_BEGIN_DECLS

/**
 * property_type:
 * @INT:    #gint type
 * @DOUBLE: #gdouble type
 * @BOOL:   #gboolean type
 * @STR:    #gchar type
 * @STRV:   array of #gchar
 *
 * Enumeration of different configuration value types
 */
typedef enum {
    INT,
    DOUBLE,
    BOOL,
    STR,
    STRV
} property_type;

/**
 * property_widget:
 * @TOGGLEBUTTON:  type of #GtkToggleButton
 * @SPINBUTTON:    type of #GtkSpinButton
 * @ENTRY:         type of #GtkEntry
 *
 * Enum defining the widget type of a configuration value
 */
typedef enum {
    TOGGLEBUTTON,
    SPINBUTTON,
    ENTRY
} property_widget;

/**
 * property_value:
 * @type:  #property_type structure
 * @wtype: #property_widget type of the property
 *
 * Configuration value structure
 */
typedef struct _property_value {
    property_type type;
    property_widget wtype;
    union {
        gint *i;
        gdouble *d;
        gboolean *b;
        gchar *s;
        gchar **v;
    } ptr;
} property_value;

/**
 * property:
 * @name:   Name of the property
 * @widget: Pointer to the #GtkWidget of the configuration value
 * @pval:   #property_value structure containing type and pointer of the property
 *
 * Configuration element structure
 */
typedef struct _property {
    const gchar *name;
    GtkWidget *widget;
    property_value pval;
} property;

void init_config(void);

void load_config(void);

void set_bool_val(const gchar *name, gboolean val);

void set_int_val(const gchar *name, gint val);

void set_prop(const gchar *name, gpointer value);

void set_prop_widget(const gchar *name, GtkWidget *widget);

gpointer get_prop(const gchar *name);

void get_prop_widget_val(gpointer key, gpointer val, gpointer data);

void get_config_value(gpointer key, gpointer val, gpointer data);

void set_config_value(gpointer key, gpointer val, gpointer data);

gboolean read_config(const gchar *filename);

gboolean write_config(const gchar *filename);

void set_default_config(void);

G_END_DECLS

#endif /* __CONFIG_H__ */

/* vim:set et sw=4 sts=4 tw=80: */
