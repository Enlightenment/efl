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

#include <stdio.h>
#include <stdlib.h>

#include "eina_hash.h"
#include "eina_suite.h"

START_TEST(eina_hash_init_shutdown)
{
   eina_hash_init();
    eina_hash_init();
    eina_hash_shutdown();
    eina_hash_init();
     eina_hash_init();
     eina_hash_shutdown();
    eina_hash_shutdown();
   eina_hash_shutdown();
}
END_TEST

START_TEST(eina_hash_simple)
{
   Eina_Hash *hash = NULL;
   int *test;
   int array[] = { 1, 42, 4, 5, 6 };

   fail_if(eina_hash_init() <= 0);

   hash = eina_hash_string_superfast_new();
   fail_if(hash == NULL);

   fail_if(eina_hash_add(hash, "1", &array[0]) != EINA_TRUE);
   fail_if(eina_hash_add(hash, "42", &array[1]) != EINA_TRUE);
   fail_if(eina_hash_direct_add(hash, "4", &array[2]) != EINA_TRUE);
   fail_if(eina_hash_direct_add(hash, "5", &array[3]) != EINA_TRUE);

   test = eina_hash_find(hash, "4");
   fail_if(!test);
   fail_if(*test != 4);

   test = eina_hash_find(hash, "42");
   fail_if(!test);
   fail_if(*test != 42);

   test = eina_hash_modify(hash, "5", &array[4]);
   fail_if(!test);
   fail_if(*test != 5);

   test = eina_hash_find(hash, "5");
   fail_if(!test);
   fail_if(*test != 6);

   fail_if(eina_hash_population(hash) != 4);

   fail_if(eina_hash_find(hash, "120") != NULL);

   fail_if(eina_hash_del(hash, "5", NULL) != EINA_TRUE);
   fail_if(eina_hash_find(hash, "5") != NULL);

   fail_if(eina_hash_del(hash, NULL, &array[2]) != EINA_TRUE);
   fail_if(eina_hash_find(hash, "4") != NULL);

   fail_if(eina_hash_del(hash, NULL, &array[2]) != EINA_FALSE);

   fail_if(eina_hash_del(hash, "1", NULL) != EINA_TRUE);
   fail_if(eina_hash_del(hash, "42", NULL) != EINA_TRUE);

   eina_hash_free(hash);

   fail_if(eina_hash_shutdown() > 0);
}
END_TEST

START_TEST(eina_hash_extended)
{
   Eina_Hash *hash = NULL;
   int i;

   fail_if(eina_hash_init() <= 0);

   hash = eina_hash_string_djb2_new();
   fail_if(hash == NULL);

   fail_if(eina_hash_direct_add(hash, "42", "42") != EINA_TRUE);

   for (i = 43; i < 3043; ++i)
     {
	char *tmp = malloc(10);
	fail_if(!tmp);
	snprintf(tmp, 10, "%i", i);
	fail_if(eina_hash_direct_add(hash, tmp, tmp) != EINA_TRUE);
     }

   fail_if(eina_hash_find(hash, "42") == NULL);

   eina_hash_free(hash);

   fail_if(eina_hash_shutdown() > 0);
}
END_TEST

void eina_test_hash(TCase *tc)
{
   tcase_add_test(tc, eina_hash_init_shutdown);
   tcase_add_test(tc, eina_hash_simple);
   tcase_add_test(tc, eina_hash_extended);
}
