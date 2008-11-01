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
#include "eina_module.h"

START_TEST(eina_module_init_shutdown)
{
   eina_module_init();
   eina_module_shutdown();
   eina_module_init();
    eina_module_init();
     eina_module_init();
     eina_module_shutdown();
     eina_module_init();
      eina_module_init();
      eina_module_shutdown();
     eina_module_shutdown();
    eina_module_shutdown();
   eina_module_shutdown();
}
END_TEST

static Eina_Bool list_cb(Eina_Module *m, void *data)
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

   eina_module_init();
   _modules = eina_module_list_get(NULL, PACKAGE_BUILD_DIR"/src/tests/", 1, &list_cb, NULL);
   fail_if(!_modules);
   eina_module_list_load(_modules);
   eina_module_list_unload(_modules);
   eina_module_list_delete(_modules);
   /* TODO delete the list */
   eina_module_shutdown();
}
END_TEST

void
eina_test_module(TCase *tc)
{
   tcase_add_test(tc, eina_module_init_shutdown);
   tcase_add_test(tc, eina_module_load_unload);
}
