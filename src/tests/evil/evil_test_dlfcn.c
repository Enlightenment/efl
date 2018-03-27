/* EVIL - EFL library for Windows port
 * Copyright (C) 2015 Vincent Torri
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

#include <Evil.h>

#include "evil_suite.h"


typedef int (*_evil_init)(void);
typedef int (*_evil_shutdwon)(void);


EFL_START_TEST(evil_dlfcn_dlopen_success)
{
   void *mod;
   int res;

   mod = dlopen("c:\\windows\\system32\\kernel32.dll", 0);
   fail_if(mod == NULL);

   res = dlclose(mod);
   fail_if(res != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_dlfcn_dlopen_failure)
{
   void *mod;

   /* non existent DLL */
   mod = dlopen("c:\\windows\\system32\\kernel32.dl", 0);
   fail_if(mod != NULL);
}
EFL_END_TEST

EFL_START_TEST(evil_dlfcn_dlsym_success)
{
   _evil_init sym_init;
   _evil_shutdwon sym_shutdown;
   void *mod;
   int res;

   mod = dlopen("libevil-1.dll", 0);
   fail_if(mod == NULL);

   sym_init = dlsym(mod, "evil_init");
   fail_if(sym_init == NULL);
   fail_if(sym_init() != 2);

   sym_shutdown = dlsym(mod, "evil_shutdown");
   fail_if(sym_shutdown == NULL);
   fail_if(sym_shutdown() != 1);

   res = dlclose(mod);
   fail_if(res != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_dlfcn_dlsym_failure)
{
   void *mod;
   void *sym;
   int res;

   mod = dlopen("libevil-1.dll", 0);
   fail_if(mod == NULL);

   /* non-existent symbol */
   sym = dlsym(mod, "evil_ini");
   fail_if(sym != NULL);

   res = dlclose(mod);
   fail_if(res != 0);
}
EFL_END_TEST

EFL_START_TEST(evil_dlfcn_dladdr)
{
   Dl_info info;
   void *mod;
   void *sym;
   char *dll;
   int res;

   mod = dlopen("libevil-1.dll", 0);
   fail_if(mod == NULL);

   sym = dlsym(mod, "evil_init");
   fail_if(sym == NULL);

   res = dladdr(sym, &info);
   fail_if(res == 0);

   fail_if(mod != info.dli_fbase);
   dll = strrchr(info.dli_fname, '\\') + 1;
   fail_if(strcmp("libevil-1.dll", dll) != 0);
   fail_if(sym != info.dli_saddr);
   fail_if(strcmp("evil_init", info.dli_sname) != 0);

   res = dlclose(mod);
   fail_if(res != 0);
}
EFL_END_TEST

void evil_test_dlfcn(TCase *tc)
{
   tcase_add_test(tc, evil_dlfcn_dlopen_success);
   tcase_add_test(tc, evil_dlfcn_dlopen_failure);
   tcase_add_test(tc, evil_dlfcn_dlsym_success);
   tcase_add_test(tc, evil_dlfcn_dlsym_failure);
   tcase_add_test(tc, evil_dlfcn_dladdr);
}
