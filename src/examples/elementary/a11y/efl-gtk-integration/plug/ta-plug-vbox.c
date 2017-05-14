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
#include "ta-plug-vbox.h"

#include "ta-plug-accessible.h"

#include <glib.h>
#include <gtk/gtk.h>

G_DEFINE_TYPE (TaPlugVBox, ta_plug_vbox, GTK_TYPE_VBOX);

static AtkObject *_get_accessible (GtkWidget *widget)
{
  static AtkObject *new = NULL;

  if (!new)
    {
      new = ta_plug_accessible_new ();
      atk_object_initialize (new, G_OBJECT (widget));
    }

  return new;
}

static void ta_plug_vbox_class_init (TaPlugVBoxClass *klass)
{
  GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;

  widget_class->get_accessible = _get_accessible;
}

static void ta_plug_vbox_init(TaPlugVBox *plug)
{
}

GtkWidget *ta_plug_vbox_new (gboolean homogeneous, gint spacing)
{
  return GTK_WIDGET (g_object_new(TA_TYPE_PLUG_VBOX,
                                  "homogeneous", homogeneous,
                                  "spacing", spacing,
                                  0));
}

gchar *ta_plug_vbox_get_id (TaPlugVBox *plug)
{
  AtkObject *plugAxObject = NULL;
  gchar *text_id = NULL;

  plugAxObject = gtk_widget_get_accessible (GTK_WIDGET (plug));
  if (ATK_IS_PLUG (plugAxObject))
    text_id = atk_plug_get_id (ATK_PLUG (plugAxObject));
  else
    g_print ("Not an instance of AtkPlug\n");

  return text_id;
}
