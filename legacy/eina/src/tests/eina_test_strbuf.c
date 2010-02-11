/* EINA - EFL data type library
 * Copyright (C) 2010 Sebastian Dransfeld
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

#include <stdio.h>

#include "eina_suite.h"
#include "Eina.h"

START_TEST(strbuf_simple)
{
   Eina_Strbuf *buf;
   char *txt;
#define TEXT "This test should be so long that it is longer than the initial size of strbuf"

   eina_init();

   buf = eina_strbuf_new();
   fail_if(!buf);

   eina_strbuf_append(buf, TEXT);
   fail_if(strcmp(eina_strbuf_string_get(buf), TEXT));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   eina_strbuf_append(buf, TEXT);
   fail_if(strcmp(eina_strbuf_string_get(buf), TEXT TEXT));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   txt = eina_strbuf_string_steal(buf);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(txt, TEXT TEXT));
   free(txt);
   fail_if(eina_strbuf_length_get(buf) != 0);
   fail_if(!strcmp(eina_strbuf_string_get(buf), TEXT TEXT));
   eina_strbuf_append(buf, TEXT);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   eina_strbuf_reset(buf);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(eina_strbuf_length_get(buf) != 0);
   fail_if(!strcmp(eina_strbuf_string_get(buf), TEXT));

   eina_strbuf_free(buf);

   eina_shutdown();
#undef TEXT
}
END_TEST

START_TEST(strbuf_remove)
{
   Eina_Strbuf *buf;

   eina_init();

   buf = eina_strbuf_new();
   fail_if(!buf);

   eina_strbuf_append(buf, "123 456 789 abc");
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   eina_strbuf_remove(buf, 0, 4);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "456 789 abc"));
   eina_strbuf_remove(buf, 8, 1000);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "456 789 "));
   eina_strbuf_remove(buf, 7, eina_strbuf_length_get(buf));
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "456 789"));
   eina_strbuf_remove(buf, 2, 4);
   fail_if(strlen(eina_strbuf_string_get(buf)) != eina_strbuf_length_get(buf));
   fail_if(strcmp(eina_strbuf_string_get(buf), "45789"));

   eina_strbuf_free(buf);

   eina_shutdown();
}
END_TEST

void
eina_test_strbuf(TCase *tc)
{
   tcase_add_test(tc, strbuf_simple);
   tcase_add_test(tc, strbuf_remove);
}
