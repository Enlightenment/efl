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
#ifndef TaPlugAccessible_h
#define TaPlugAccessible_h

#include <atk/atk.h>

G_BEGIN_DECLS

#define TA_TYPE_PLUG_ACCESSIBLE                      (ta_plug_accessible_get_type ())
#define TA_PLUG_ACCESSIBLE(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), TA_TYPE_PLUG_ACCESSIBLE, TaPlugAccessible))
#define TA_PLUG_ACCESSIBLE_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST ((klass), TA_TYPE_PLUG_ACCESSIBLE, TaPlugAccessibleClass))
#define TA_IS_PLUG_ACCESSIBLE(obj)                   (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TA_TYPE_PLUG_ACCESSIBLE))
#define TA_IS_PLUG_ACCESSIBLE_CLASS(klass)           (G_TYPE_CHECK_CLASS_TYPE ((klass), TA_TYPE_PLUG_ACCESSIBLE))
#define TA_PLUG_ACCESSIBLE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TA_TYPE_PLUG_ACCESSIBLE, TaPlugAccessibleClass))

typedef struct _TaPlugAccessible                TaPlugAccessible;
typedef struct _TaPlugAccessibleClass           TaPlugAccessibleClass;
typedef struct _TaPlugAccessiblePrivate         TaPlugAccessiblePrivate;

struct _TaPlugAccessible {
    AtkPlug parent;
};

struct _TaPlugAccessibleClass {
    AtkPlugClass parentClass;
};

GType ta_plug_accessible_get_type(void) G_GNUC_CONST;

AtkObject *ta_plug_accessible_new (void);

G_END_DECLS

#endif // TaPlugAccessible_h
