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
#include <stdio.h>
#include <time.h>

#ifdef EINA_BENCH_HAVE_GLIB
# include <glib.h>
#endif

#include "Evas_Data.h"
#include "Ecore_Data.h"

#include "Eina.h"

#if EINA_ENABLE_BENCH_E17

typedef struct _Eina_Stringshare_Test Eina_Stringshare_Test;
struct _Eina_Stringshare_Test
{
   const char *name;

   int (*init)(void);
   const char *(*add)(const char *str);
   void (*del)(const char *str);
   int (*shutdown)(void);
};

static Eina_Stringshare_Test eina_str = {
   "eina",
   eina_init,
   eina_stringshare_add,
   eina_stringshare_del,
   eina_shutdown
};

static Eina_Stringshare_Test evas_str = {
   "evas",
   NULL,
   evas_stringshare_add,
   evas_stringshare_del,
   NULL
};

static Eina_Stringshare_Test ecore_str = {
   "ecore",
   ecore_string_init,
   ecore_string_instance,
   ecore_string_release,
   ecore_string_shutdown
};

static Eina_Stringshare_Test *tests[] = {
   &eina_str,
   &evas_str,
   &ecore_str,
   NULL
};

static void
eina_bench_e17_stringshare(Eina_Stringshare_Test *str)
{
   Eina_Counter *cnt;
   char *result;

   cnt = eina_counter_new(str->name);

   eina_counter_start(cnt);

   if (str->init)
     str->init();

//#include "strlog"

   if (str->shutdown)
     str->shutdown();

   eina_counter_stop(cnt, 1);

   result = eina_counter_dump(cnt);
   fprintf(stderr, "For `%s`:\n%s\n", str->name, result);
   free(result);

   eina_counter_free(cnt);
}
#endif

void
eina_bench_e17(void)
{
#if EINA_ENABLE_BENCH_E17
   int i;

   eina_init();

   for (i = 0; tests[i]; ++i)
      eina_bench_e17_stringshare(tests[i]);

   eina_shutdown();
#endif
}
