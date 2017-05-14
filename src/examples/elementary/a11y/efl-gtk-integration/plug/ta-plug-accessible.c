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
#include "ta-plug-accessible.h"

#include <atk/atk.h>
#include <glib.h>
#include <gtk/gtk.h>

struct _TaPlugAccessiblePrivate {
  GtkWidget *widget;
};

#define TA_PLUG_ACCESSIBLE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), TA_TYPE_PLUG_ACCESSIBLE, TaPlugAccessiblePrivate))

G_DEFINE_TYPE (TaPlugAccessible, ta_plug_accessible, ATK_TYPE_PLUG);

static void ta_plug_accessible_initialize (AtkObject *object, gpointer data)
{
  g_return_if_fail (TA_IS_PLUG_ACCESSIBLE (object));

  TaPlugAccessible *plug = NULL;
  TaPlugAccessiblePrivate *priv = NULL;

  ATK_OBJECT_CLASS(ta_plug_accessible_parent_class)->initialize(object, data);

  plug = TA_PLUG_ACCESSIBLE (object);
  priv = TA_PLUG_ACCESSIBLE_GET_PRIVATE(plug);
  priv->widget = NULL;
  if (GTK_IS_WIDGET (data))
    priv->widget = GTK_WIDGET (data);
}

static const gchar *ta_plug_accessible_get_name (AtkObject *object)
{
  g_return_val_if_fail(ATK_IS_OBJECT (object), NULL);
  return "The Plug";
}

static AtkRole ta_plug_accessible_get_role (AtkObject *object)
{
  g_return_val_if_fail(ATK_IS_OBJECT (object), ATK_ROLE_UNKNOWN);
  return ATK_ROLE_PANEL;
}

static gint ta_plug_accessible_get_n_children (AtkObject *object)
{
  g_return_val_if_fail (TA_IS_PLUG_ACCESSIBLE (object), 0);

  TaPlugAccessible *plug = NULL;
  TaPlugAccessiblePrivate *priv = NULL;
  GList *children = NULL;
  GList *item = NULL;
  gint n_children = 0;

  plug = TA_PLUG_ACCESSIBLE (object);
  priv = TA_PLUG_ACCESSIBLE_GET_PRIVATE(plug);

  if (!priv->widget)
    return 0;

  if (!GTK_IS_CONTAINER (priv->widget))
    return 0;

  children = gtk_container_get_children (GTK_CONTAINER (priv->widget));
  n_children = g_list_length (children);
  g_list_free (children);

  return n_children;
}

static AtkObject *ta_plug_accessible_ref_child (AtkObject *object, gint i)
{
  g_return_val_if_fail (TA_IS_PLUG_ACCESSIBLE (object), NULL);

  TaPlugAccessible *plug = NULL;
  TaPlugAccessiblePrivate *priv = NULL;
  GList *children = NULL;
  GList *item = NULL;
  gint n_children = 0;
  GtkWidget *child = NULL;
  AtkObject *axChild = NULL;

  plug = TA_PLUG_ACCESSIBLE (object);
  priv = TA_PLUG_ACCESSIBLE_GET_PRIVATE(plug);

  if (!priv->widget)
    return NULL;

  if (!GTK_IS_CONTAINER (priv->widget))
    return NULL;

  children = gtk_container_get_children (GTK_CONTAINER (priv->widget));
  if (i < 0 || i >= g_list_length (children))
    return NULL;

  child = GTK_WIDGET (g_list_nth_data (children, i));
  if (!child)
    return NULL;

  g_list_free (children);

  axChild = gtk_widget_get_accessible (child);
  return g_object_ref (axChild);
}

static void ta_plug_accessible_dispose (GObject *object)
{
  TaPlugAccessible *plug = NULL;
  TaPlugAccessiblePrivate *priv = NULL;

  plug = TA_PLUG_ACCESSIBLE (object);
  priv = TA_PLUG_ACCESSIBLE_GET_PRIVATE(plug);

  if (priv->widget)
    {
      g_object_unref (priv->widget);
      priv->widget = NULL;
    }

  G_OBJECT_CLASS(ta_plug_accessible_parent_class)->dispose(object);
}

static void ta_plug_accessible_class_init (TaPlugAccessibleClass *klass)
{
  GObjectClass *g_object_class = G_OBJECT_CLASS(klass);
  AtkObjectClass *atk_object_class = NULL;

  g_object_class->dispose = ta_plug_accessible_dispose;

  atk_object_class = ATK_OBJECT_CLASS (klass);
  atk_object_class->initialize = ta_plug_accessible_initialize;
  atk_object_class->get_name = ta_plug_accessible_get_name;
  atk_object_class->get_role = ta_plug_accessible_get_role;
  atk_object_class->get_n_children = ta_plug_accessible_get_n_children;
  atk_object_class->get_index_in_parent = NULL;
  atk_object_class->ref_child = ta_plug_accessible_ref_child;

  g_type_class_add_private(g_object_class, sizeof(TaPlugAccessiblePrivate));
}

static void ta_plug_accessible_init(TaPlugAccessible *plug)
{
}

AtkObject *ta_plug_accessible_new ()
{
  return ATK_OBJECT (g_object_new(TA_TYPE_PLUG_ACCESSIBLE, 0));
}
