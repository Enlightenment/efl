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
      eina_module_root_add(PACKAGE_BUILD_DIR);
      eina_module_shutdown();
     eina_module_shutdown();
    eina_module_shutdown();
   eina_module_shutdown();
}
END_TEST

static int i42 = 42;
static Eina_Module_Export static_test = { "simple", NULL, "test", &i42 };
EAPI int stupid_test = 7;

static Eina_Bool
_eina_module_test_cb(__UNUSED__ Eina_Module *m, __UNUSED__ void *data)
{
   return EINA_TRUE;
}

START_TEST(eina_module_simple)
{
   Eina_Module_Group *gp;
   Eina_Module *m;
   Eina_List *list;

   eina_module_init();

   eina_module_root_add(PACKAGE_BUILD_DIR"/src/tests");

   gp = eina_module_group_new();
   fail_if(!gp);

   eina_module_path_register(gp, PACKAGE_BUILD_DIR"/src/modules", EINA_TRUE);
   eina_module_path_register(gp, PACKAGE_BUILD_DIR"/src/lib", EINA_FALSE);
   eina_module_path_register(gp, PACKAGE_BUILD_DIR"/src/lib", EINA_FALSE);

   eina_module_app_register(gp, "eina", "test", NULL);
   eina_module_app_register(gp, "eina", "bench", "1.0.0");

   eina_module_register(gp, &static_test);

   m = eina_module_new(gp, "unknown");
   fail_if(m);

   m = eina_module_new(gp, "simple");
   fail_if(!m);

   fail_if(eina_module_load(m) != EINA_TRUE);

   fprintf(stderr, "path: %s\n", eina_module_path_get(m));
   fail_if(strcmp("test", eina_module_export_type_get(m)));
   fail_if(eina_module_export_version_get(m) != NULL);
   fail_if(strcmp("simple", eina_module_export_name_get(m)));
   fail_if(eina_module_export_object_get(m) != &i42);
   fail_if(eina_module_symbol_get(m, "eina_list_init") != &eina_list_init);

   eina_module_unload(m);
   eina_module_delete(m);

   list = eina_module_list_new(gp, _eina_module_test_cb, NULL);
   eina_module_list_load(list);
   eina_module_list_unload(list);
   eina_module_list_delete(list);

   m = eina_module_new(gp, "simple");

   eina_module_group_delete(gp);

   eina_module_shutdown();
}
END_TEST

void
eina_test_module(TCase *tc)
{
   tcase_add_test(tc, eina_module_init_shutdown);
   tcase_add_test(tc, eina_module_simple);
}
