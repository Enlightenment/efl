/* EINA - EFL data type library
 * Copyright (C) 2012 Cedric Bail
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

#include <Eina.h>

#include "eina_suite.h"

typedef struct _Eina_Cow_Test Eina_Cow_Test;
struct _Eina_Cow_Test
{
   unsigned char c;
   int i;
   void *d;
};

static void
_eina_test_log(const Eina_Log_Domain *d EINA_UNUSED,
               Eina_Log_Level level EINA_UNUSED, const char *file EINA_UNUSED, const char *fnc EINA_UNUSED, int line EINA_UNUSED,
               const char *fmt EINA_UNUSED, void *data, va_list args EINA_UNUSED)
{
   Eina_Bool *bol = data;

   *bol = EINA_TRUE;
}

EFL_START_TEST(eina_cow_bad)
{
   const Eina_Cow_Test *cur;
   Eina_Cow_Test *write;
   Eina_Cow *cow;
   Eina_Cow_Test default_value = { 7, 42, NULL };

   cow = eina_cow_add("COW Test", sizeof (Eina_Cow_Test), 16, &default_value, EINA_TRUE);
   fail_if(cow == NULL);

   cur = eina_cow_alloc(cow);
   fail_if(cur == NULL);

   write = eina_cow_write(cow, (const Eina_Cow_Data**) &cur);
   fail_if(write == NULL || write == &default_value);

   write->i = 7;
   eina_cow_done(cow, (const Eina_Cow_Data**) &cur, write, EINA_FALSE);
   fail_if(cur->i != 7 || default_value.i != 42);

#ifdef EINA_COW_MAGIC
   Eina_Bool over_commit = EINA_FALSE;
   Eina_Bool over_writing = EINA_FALSE;

   eina_log_print_cb_set(_eina_test_log, &over_commit);
   /* Testing over commit */
   eina_cow_done(cow, (const Eina_Cow_Data**) &cur, write, EINA_FALSE);
   fail_if(!over_commit);

   write = eina_cow_write(cow, (const Eina_Cow_Data**) &cur);
   fail_if(write == NULL || write == &default_value);

   eina_log_print_cb_set(_eina_test_log, &over_writing);
   /* Testing over writing */
   write = eina_cow_write(cow, (const Eina_Cow_Data**) &cur);
   fail_if(write != NULL || !over_writing);
#else
   (void) _eina_test_log;
#endif

   eina_cow_free(cow, (const Eina_Cow_Data**) &cur);

   eina_cow_del(cow);
}
EFL_END_TEST

EFL_START_TEST(eina_cow)
{
   const Eina_Cow_Test *prev;
   const Eina_Cow_Test *cur;
   Eina_Cow_Test *write;
   Eina_Cow *cow;
   Eina_Cow_Test default_value = { 42, 0, NULL };

   cow = eina_cow_add("COW Test", sizeof (Eina_Cow_Test), 16, &default_value, EINA_TRUE);
   fail_if(cow == NULL);

   prev = eina_cow_alloc(cow);
   cur = eina_cow_alloc(cow);
   fail_if(prev == NULL || cur == NULL);

   write = eina_cow_write(cow, (const Eina_Cow_Data**) &cur);
   fail_if(write == NULL || write == &default_value);

   write->i = 7;
   eina_cow_done(cow, (const Eina_Cow_Data**) &cur, write, EINA_TRUE);
   fail_if(cur->i != 7 || prev->i != 0);

   eina_cow_memcpy(cow,
                   (const Eina_Cow_Data**) &prev,
                   (const Eina_Cow_Data*) cur);
   fail_if(cur->i != 7 || prev->i != 7);
   fail_if(default_value.i != 0);

   write = eina_cow_write(cow, (const Eina_Cow_Data**) &cur);
   fail_if(write == NULL || write == &default_value);

   write->i = 42; write->c = 5;
   eina_cow_done(cow, (const Eina_Cow_Data**) &cur, write, EINA_TRUE);
   fail_if(cur->i != 42 || cur->c != 5 ||
           prev->i != 7 || prev->c != 42 ||
           default_value.c != 42 || default_value.i != 0);

   fail_if(eina_cow_gc(cow) == EINA_FALSE);

   write = eina_cow_write(cow, (const Eina_Cow_Data**) &cur);
   write->i = 7; write->c = 42;
   eina_cow_done(cow, (const Eina_Cow_Data**) &cur, write, EINA_TRUE);

   write = eina_cow_write(cow, (const Eina_Cow_Data**) &prev);
   eina_cow_done(cow, (const Eina_Cow_Data**) &prev, write, EINA_TRUE);

   fail_if(eina_cow_gc(cow) == EINA_FALSE);
   fail_if(eina_cow_gc(cow) == EINA_FALSE);
   fail_if(cur != prev);

   eina_cow_free(cow, (const Eina_Cow_Data**) &cur);
   eina_cow_free(cow, (const Eina_Cow_Data**) &prev);

   eina_cow_del(cow);
}
EFL_END_TEST

void
eina_test_cow(TCase *tc)
{
   tcase_add_test(tc, eina_cow);
   tcase_add_test(tc, eina_cow_bad);
}
