/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#ifdef EINA_BENCH_HAVE_GLIB
# include <glib.h>
#endif

#ifdef EINA_BENCH_HAVE_EVAS
# include <Evas.h>
#endif

#ifdef EINA_BENCH_HAVE_ECORE
# include <Ecore_Data.h>
#endif

#include "eina_stringshare.h"
#include "eina_counter.h"

#if EINA_ENABLE_BENCH_E17

typedef struct _Eina_Stringshare_Test Eina_Stringshare_Test;
struct _Eina_Stringshare_Test
{
   const char *name;

   int (*init)(void);
   const char *(*add)(const char* str);
   void (*del)(const char* str);
   int (*shutdown)(void);
};

static const char *strings[30000];
static Eina_Stringshare_Test eina_str = {
  "eina",
  eina_stringshare_init,
  eina_stringshare_add,
  eina_stringshare_del,
  eina_stringshare_shutdown
};

#ifdef EINA_BENCH_HAVE_EVAS
static Eina_Stringshare_Test evas_str = {
  "evas",
  evas_init,
  evas_stringshare_add,
  evas_stringshare_del,
  evas_shutdown
};
#endif

#ifdef EINA_BENCH_HAVE_ECORE
static Eina_Stringshare_Test ecore_str = {
  "ecore",
  ecore_string_init,
  ecore_string_instance,
  ecore_string_release,
  ecore_string_shutdown
};
#endif

static Eina_Stringshare_Test* str[] = {
  &eina_str,
#ifdef EINA_BENCH_HAVE_EVAS
  &evas_str,
#endif
#ifdef EINA_BENCH_HAVE_ECORE
  &ecore_str,
#endif
  NULL
};

static void
eina_bench_e17_stringshare(Eina_Stringshare_Test *str)
{
   Eina_Counter *cnt;

   cnt = eina_counter_add(str->name);

   eina_counter_start(cnt);

   str->init();

#include "strlog"

   str->shutdown();

   eina_counter_stop(cnt, 1);

   fprintf(stderr, "For `%s`:\n", str->name);
   eina_counter_dump(cnt, stderr);

   eina_counter_delete(cnt);
}
#endif

void
eina_bench_e17(void)
{
#if EINA_ENABLE_BENCH_E17
   int i;

   eina_counter_init();

   for (i = 0; str[i]; ++i)
     eina_bench_e17_stringshare(str[i]);

   eina_counter_shutdown();
#endif
}
