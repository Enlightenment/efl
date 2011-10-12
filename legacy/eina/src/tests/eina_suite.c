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

#include "eina_suite.h"
#include "Eina.h"
#include <stdio.h>
#include <string.h>

typedef struct _Eina_Test_Case Eina_Test_Case;
struct _Eina_Test_Case
{
   const char *test_case;
   void (*build)(TCase *tc);
};

static const Eina_Test_Case etc[] = {
   { "FixedPoint", eina_test_fp },
   { "Array", eina_test_array },
   { "Binary Share", eina_test_binshare },
   { "String Share", eina_test_stringshare },
   { "UString Share", eina_test_ustringshare },
   { "Log", eina_test_log },
   { "Error", eina_test_error },
   { "Magic", eina_test_magic },
   { "Inlist", eina_test_inlist },
   { "Lazy alloc", eina_test_lalloc },
   { "Main", eina_test_main },
   { "Counter", eina_test_counter },
   { "Hash", eina_test_hash },
   { "List", eina_test_list },
   { "CList", eina_test_clist },
   { "Iterator", eina_test_iterator },
   { "Accessor", eina_test_accessor },
   { "Module", eina_test_module },
   { "Convert", eina_test_convert },
   { "Rbtree", eina_test_rbtree },
   { "File", eina_test_file },
   { "Benchmark", eina_test_benchmark },
   { "Mempool", eina_test_mempool },
   { "Rectangle", eina_test_rectangle },
   { "Matrix Sparse", eina_test_matrixsparse },
   { "Eina Tiler", eina_test_tiler },
   { "Eina Strbuf", eina_test_strbuf },
   { "Eina Binbuf", eina_test_binbuf },
   { "String", eina_test_str },
   { "Unicode String", eina_test_ustr },
   { "QuadTree", eina_test_quadtree },
   { "Sched", eina_test_sched },
   { "Simple Xml Parser", eina_test_simple_xml_parser},
   { NULL, NULL }
};

static void
_list_tests(void)
{
   const Eina_Test_Case *itr = etc;
      fputs("Available Test Cases:\n", stderr);
   for (; itr->test_case; itr++)
      fprintf(stderr, "\t%s\n", itr->test_case);
}

static Eina_Bool
_use_test(int argc, const char **argv, const char *test_case)
{
   if (argc < 1)
      return 1;

   for (; argc > 0; argc--, argv++)
      if (strcmp(test_case, *argv) == 0)
         return 1;

   return 0;
}

Suite *
eina_build_suite(int argc, const char **argv)
{
   TCase *tc;
   Suite *s;
   int i;

   s = suite_create("Eina");

   for (i = 0; etc[i].test_case; ++i)
     {
        if (!_use_test(argc, argv, etc[i].test_case))
           continue;

        tc = tcase_create(etc[i].test_case);

        etc[i].build(tc);

        suite_add_tcase(s, tc);
        tcase_set_timeout(tc, 0);
     }

   return s;
}

/* FIXME this is a copy from eina_test_mempool
 * we should remove the duplication
 */
static Eina_Array *_modules;
static void _mempool_init(void)
{
   eina_init();
   /* force modules to be loaded in case they are not installed */
   _modules = eina_module_list_get(NULL,
                                   PACKAGE_BUILD_DIR "/src/modules",
                                   EINA_TRUE,
                                   NULL,
                                   NULL);
   eina_module_list_load(_modules);
}

static void _mempool_shutdown(void)
{
   eina_module_list_free(_modules);
   /* TODO delete the list */
   eina_shutdown();
}

int
main(int argc, char **argv)
{
   Suite *s;
   SRunner *sr;
   int i, failed_count;

   for (i = 1; i < argc; i++)
      if ((strcmp(argv[i], "-h") == 0) ||
          (strcmp(argv[i], "--help") == 0))
        {
           fprintf(stderr, "Usage:\n\t%s [test_case1 .. [test_caseN]]\n",
                   argv[0]);
           _list_tests();
           return 0;
        }
      else if ((strcmp(argv[i], "-l") == 0) ||
               (strcmp(argv[i], "--list") == 0))
        {
           _list_tests();
           return 0;
        }

   s = eina_build_suite(argc - 1, (const char **)argv + 1);
   sr = srunner_create(s);

   _mempool_init();

   srunner_run_all(sr, CK_ENV);
   failed_count = srunner_ntests_failed(sr);
   srunner_free(sr);

   _mempool_shutdown();

   return (failed_count == 0) ? 0 : 255;
}
