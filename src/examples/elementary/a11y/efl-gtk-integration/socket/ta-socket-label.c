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
#include "ta-socket-label.h"

#include "ta-socket-accessible.h"

#include <glib.h>
#include <gtk/gtk.h>

G_DEFINE_TYPE (TaSocketLabel, ta_socket_label, GTK_TYPE_LABEL);


static AtkObject *_get_accessible (GtkWidget *widget)
{
  static AtkObject *new = NULL;

  if (!new)
    {
      new = ta_socket_accessible_new ();
      atk_object_initialize (new, G_OBJECT (widget));
    }

  return new;
}

static void ta_socket_label_class_init (TaSocketLabelClass *klass)
{
  GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;

  widget_class->get_accessible = _get_accessible;
}

static void ta_socket_label_init(TaSocketLabel *socket)
{
}

GtkWidget *ta_socket_label_new (const gchar *text)
{
  return GTK_WIDGET (g_object_new(TA_TYPE_SOCKET_LABEL,
                                  "label", text,
                                  0));
}
