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

#include <stdlib.h>

#include "eina_suite.h"
#include "eina_magic.h"

START_TEST(eina_magic_string_init_shutdown)
{
   eina_magic_string_init();
    eina_magic_string_init();
    eina_magic_string_shutdown();
    eina_magic_string_init();
     eina_magic_string_init();
     eina_magic_string_shutdown();
    eina_magic_string_shutdown();
   eina_magic_string_shutdown();
}
END_TEST

#define EINA_MAGIC_TEST 0x7781fee7
#define EINA_MAGIC_TEST2 0x42241664
#define EINA_MAGIC_STRING "Eina Magic Test"

typedef struct _Eina_Magic_Struct Eina_Magic_Struct;
struct _Eina_Magic_Struct
{
   EINA_MAGIC
};

START_TEST(eina_magic_simple)
{
   Eina_Magic_Struct *ems = NULL;

   eina_magic_string_init();

   eina_magic_string_set(EINA_MAGIC_TEST, EINA_MAGIC_STRING);
   eina_magic_string_set(EINA_MAGIC_TEST2, NULL);
   eina_magic_string_set(EINA_MAGIC_TEST2, NULL);
   eina_magic_string_set(EINA_MAGIC_TEST2, EINA_MAGIC_STRING);

   fail_if(eina_magic_string_get(EINA_MAGIC_TEST) == NULL);
   fail_if(strcmp(eina_magic_string_get(EINA_MAGIC_TEST), EINA_MAGIC_STRING) != 0);

   fail_if(EINA_MAGIC_CHECK(ems, EINA_MAGIC_TEST));
   EINA_MAGIC_FAIL(ems, EINA_MAGIC_TEST);

   ems = malloc(sizeof (Eina_Magic_Struct));
   fail_if(!ems);
   EINA_MAGIC_SET(ems, EINA_MAGIC_TEST);

   fail_if(!EINA_MAGIC_CHECK(ems, EINA_MAGIC_TEST));

   EINA_MAGIC_SET(ems, EINA_MAGIC_NONE);
   EINA_MAGIC_FAIL(ems, EINA_MAGIC_TEST);

   EINA_MAGIC_SET(ems, 42424242);
   EINA_MAGIC_FAIL(ems, EINA_MAGIC_TEST);

   eina_magic_string_shutdown();
}
END_TEST

void eina_test_magic(TCase *tc)
{
   tcase_add_test(tc, eina_magic_string_init_shutdown);
   tcase_add_test(tc, eina_magic_simple);
}

