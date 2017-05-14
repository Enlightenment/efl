/*
 * Copyright (C) 2011 Igalia S.L.
 * Authors: Mario Sanchez Prada <msanchez@igalia.com>
 *
 * Based on a C# example written in C# by Mike Gorse:
 * http://mgorse.freeshell.org/plug-socket-test.tar.gz
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 3 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */
#ifndef TaPlugVBox_h
#define TaPlugVBox_h

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define TA_TYPE_PLUG_VBOX            (ta_plug_vbox_get_type ())
#define TA_PLUG_VBOX(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TA_TYPE_PLUG_VBOX, TaPlugVBox))
#define TA_PLUG_VBOX_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TA_TYPE_PLUG_VBOX, TaPlugVBoxClass))
#define TA_IS_PLUG_VBOX(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TA_TYPE_PLUG_VBOX))
#define TA_IS_PLUG_VBOX_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TA_TYPE_PLUG_VBOX))
#define TA_PLUG_VBOX_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), TA_TYPE_PLUG_VBOX, TaPlugVBoxClass))

typedef struct _TaPlugVBox                TaPlugVBox;
typedef struct _TaPlugVBoxClass           TaPlugVBoxClass;
typedef struct _TaPlugVBoxPrivate         TaPlugVBoxPrivate;

struct _TaPlugVBox {
    GtkVBox parent;
};

struct _TaPlugVBoxClass {
    GtkVBoxClass parentClass;
};

GType ta_plug_vbox_get_type(void) G_GNUC_CONST;

GtkWidget *ta_plug_vbox_new (gboolean homogeneous, gint spacing);

gchar *ta_plug_vbox_get_id (TaPlugVBox *plug);

G_END_DECLS

#endif // TaPlugVBox_h
