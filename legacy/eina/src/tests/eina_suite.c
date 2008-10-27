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

#include "eina_suite.h"
#include "eina_mempool.h"

typedef struct _Eina_Test_Case Eina_Test_Case;
struct _Eina_Test_Case
{
   const char *test_case;
   void (*build)(TCase *tc);
};

static const Eina_Test_Case etc[] = {
  { "Array", eina_test_array },
  { "String Share", eina_test_stringshare },
  { "Error", eina_test_error },
  { "Magic", eina_test_magic },
  { "Inlist", eina_test_inlist },
  { "Lazy alloc", eina_test_lalloc },
  { "Main", eina_test_main },
  { "Counter", eina_test_counter },
  { "Hash", eina_test_hash },
  { "List", eina_test_list },
  { "Iterator", eina_test_iterator },
  { "Accessor", eina_test_accessor },
  { "Module", eina_test_module },
  { "Convert", eina_test_convert },
  { "Rbtree", eina_test_rbtree },
  { "File", eina_test_file },
  { "Benchmark", eina_test_benchmark },
  { "Mempool", eina_test_mempool },
  { "Rectangle", eina_test_rectangle },
  { NULL, NULL }
};

Suite *
eina_build_suite(void)
{
   TCase *tc;
   Suite *s;
   int i;

   s = suite_create("Eina");

   for (i = 0; etc[i].test_case != NULL; ++i)
     {
	tc = tcase_create(etc[i].test_case);

	etc[i].build(tc);

	suite_add_tcase(s, tc);
     }

   return s;
}

/* FIXME this is a copy from eina_test_mempool
 * we should remove the duplication
 */
static Eina_Array *_modules;
static void _mempool_init(void)
{
    eina_mempool_init();
    /* force modules to be loaded in case they are not installed */
    _modules = eina_module_list_get(PACKAGE_BUILD_DIR"/src/modules", 1, NULL, NULL);
    eina_module_list_load(_modules);
}

static void _mempool_shutdown(void)
{
   eina_module_list_delete(_modules);
   /* TODO delete the list */
   eina_mempool_shutdown();
}

int
main(void)
{
   Suite *s;
   SRunner *sr;
   int failed_count;


   s = eina_build_suite();
   sr = srunner_create(s);

   _mempool_init();

   srunner_run_all(sr, CK_NORMAL);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   _mempool_shutdown();

   return (failed_count == 0) ? 0 : 255;
}
