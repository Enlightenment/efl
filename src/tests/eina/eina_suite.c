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

#include <Eina.h>

#include "eina_suite.h"
#include "../efl_check.h"

static const Efl_Test_Case etc[] = {
   { "FixedPoint", eina_test_fp },
   { "Inarray", eina_test_inarray },
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
   { "Value", eina_test_value },
   { "COW", eina_test_cow },
   // Disabling Eina_Model test
   //   { "Model", eina_test_model },
   { "Barrier", eina_test_barrier },
   { "Tmp String", eina_test_tmpstr },
   { "Locking", eina_test_locking },
   { "ABI", eina_test_abi },
   { "Trash", eina_test_trash },
#ifdef XATTR_TEST_DIR
   { "Xattr", eina_test_xattr },
#endif
   { "Crc", eina_test_crc },
   { "Quad", eina_test_quad },
   { "Matrix", eina_test_matrix },
   { "Quaternion", eina_test_quaternion },
   { "Vector", eina_test_vector },
   { "Promise", eina_test_promise },
   { "Bezier", eina_test_bezier },
   { "SafePointer", eina_test_safepointer },
   { "Slice", eina_test_slice },
   { NULL, NULL }
};

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
   unsigned int i;
   Eina_Array_Iterator it;
   Eina_Module *module;
   eina_module_list_free(_modules);
   if (_modules)
     {
        EINA_ARRAY_ITER_NEXT(_modules, i, module, it)
          free(module);
        eina_array_free(_modules);
     }
   eina_shutdown();
}

int
main(int argc, char **argv)
{
   int failed_count;

   if (!_efl_test_option_disp(argc, argv, etc))
     return 0;

   putenv("EFL_RUN_IN_TREE=1");

   _mempool_init();

   failed_count = _efl_suite_build_and_run(argc - 1, (const char **)argv + 1,
                                           "Eina", etc);

   _mempool_shutdown();

   return (failed_count == 0) ? 0 : 255;
}
