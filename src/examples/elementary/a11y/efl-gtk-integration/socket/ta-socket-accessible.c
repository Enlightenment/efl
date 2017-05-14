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
#include "ta-socket-accessible.h"

#include <atk/atk.h>
#include <glib.h>
#include <gtk/gtk.h>

G_DEFINE_TYPE (TaSocketAccessible, ta_socket_accessible, ATK_TYPE_SOCKET);

static const gchar *ta_socket_accessible_get_name (AtkObject *object)
{
  g_return_val_if_fail(ATK_IS_OBJECT (object), NULL);
  return "The Socket";
}

static AtkRole ta_socket_accessible_get_role (AtkObject *object)
{
  g_return_val_if_fail(ATK_IS_OBJECT (object), ATK_ROLE_UNKNOWN);
  return ATK_ROLE_PANEL;
}

static void ta_socket_accessible_class_init (TaSocketAccessibleClass *klass)
{
  AtkObjectClass *atk_object_class = NULL;

  atk_object_class = ATK_OBJECT_CLASS (klass);
  atk_object_class->get_name = ta_socket_accessible_get_name;
  atk_object_class->get_role = ta_socket_accessible_get_role;
}

static void ta_socket_accessible_init(TaSocketAccessible *socket)
{
}

AtkObject *ta_socket_accessible_new (void)
{
  return ATK_OBJECT (g_object_new(TA_TYPE_SOCKET_ACCESSIBLE, 0));
}
