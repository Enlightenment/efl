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

#include <stdio.h>

#include "eina_suite.h"
#include "Eina.h"

static Eina_Bool list_cb(Eina_Module *m, void *data EINA_UNUSED)
{
   int *sym;
   const char *file;

   /* the reference count */
   eina_module_load(m);
   /* get */
   sym = eina_module_symbol_get(m, "dummy_symbol");
   fail_if(!sym);
   fail_if(*sym != 0xbad);
   file = eina_module_file_get(m);
   fail_if(!file);
   eina_module_unload(m);

   return EINA_TRUE;
}


START_TEST(eina_module_load_unload)
{
   Eina_Array *_modules;
   unsigned int i;
   Eina_Array_Iterator it;
   Eina_Module *module;	
   
   eina_init();
   _modules = eina_module_list_get(NULL,
                                   PACKAGE_BUILD_DIR "/src/tests/",
                                   EINA_TRUE,
                                   &list_cb,
                                   NULL);
   fail_if(!_modules);
   eina_module_list_load(_modules);
   eina_module_list_unload(_modules);
   eina_module_list_free(_modules);
   EINA_ARRAY_ITER_NEXT(_modules, i, module, it)
     free(module);
   eina_array_free(_modules);
   eina_shutdown();
}
END_TEST

START_TEST(eina_module_find_test)
{
   Eina_Array *_modules;

   eina_init();
   _modules = eina_module_list_get(NULL,
                                   PACKAGE_BUILD_DIR "/src/tests/",
                                   EINA_TRUE,
                                   &list_cb,
                                   NULL);
   fail_if(!_modules);
   fail_if(eina_module_find(_modules, NULL) != NULL);
   eina_shutdown();
}
END_TEST

void
eina_test_module(TCase *tc)
{
   tcase_add_test(tc, eina_module_load_unload);
   tcase_add_test(tc, eina_module_find_test);
}
