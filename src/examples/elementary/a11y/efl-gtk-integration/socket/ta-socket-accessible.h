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
#ifndef TaSocketAccessible_h
#define TaSocketAccessible_h

#include <atk/atk.h>

G_BEGIN_DECLS

#define TA_TYPE_SOCKET_ACCESSIBLE                      (ta_socket_accessible_get_type ())
#define TA_SOCKET_ACCESSIBLE(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), TA_TYPE_SOCKET_ACCESSIBLE, TaSocketAccessible))
#define TA_SOCKET_ACCESSIBLE_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass), TA_TYPE_SOCKET_ACCESSIBLE, TaSocketAccessibleClass))
#define TA_IS_SOCKET_ACCESSIBLE(obj)                   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TA_TYPE_SOCKET_ACCESSIBLE))
#define TA_IS_SOCKET_ACCESSIBLE_CLASS(klass)           (G_TYPE_CHECK_CLASS_TYPE ((klass), TA_TYPE_SOCKET_ACCESSIBLE))
#define TA_SOCKET_ACCESSIBLE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TA_TYPE_SOCKET_ACCESSIBLE, TaSocketAccessibleClass))

typedef struct _TaSocketAccessible                TaSocketAccessible;
typedef struct _TaSocketAccessibleClass           TaSocketAccessibleClass;
typedef struct _TaSocketAccessiblePrivate         TaSocketAccessiblePrivate;

struct _TaSocketAccessible {
    AtkSocket parent;
};

struct _TaSocketAccessibleClass {
    AtkSocketClass parentClass;
};

GType ta_socket_accessible_get_type(void) G_GNUC_CONST;

AtkObject *ta_socket_accessible_new (void);

G_END_DECLS

#endif // TaSocketAccessible_h
