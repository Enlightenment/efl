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

static void
_eina_mempool_test(Eina_Mempool *mp,
                   Eina_Bool with_realloc, Eina_Bool with_gc, Eina_Bool accurate_from)
{
   int *tbl[512];
   int i;

        fail_if(!mp);

   for (i = 0; i < 512; ++i)
     {
        tbl[i] = eina_mempool_malloc(mp, sizeof (int));
        fail_if(!tbl[i]);
        if (accurate_from)
          fail_if(eina_mempool_from(mp, tbl[i]) != EINA_TRUE);
        *tbl[i] = i;
     }

   for (i = 0; i < 512; ++i)
        fail_if(*tbl[i] != i);

   for (i = 0; i < 256; ++i)
     {
        eina_mempool_free(mp, tbl[i]);
        if (accurate_from)
          fail_if(eina_mempool_from(mp, tbl[i]) != EINA_FALSE);
     }

   if (with_realloc)
      fail_if(eina_mempool_realloc(mp, tbl[500], 25) == NULL);
   else
      fail_if(eina_mempool_realloc(mp, tbl[500], 25) != NULL);

   if (with_gc)
     {
        eina_mempool_gc(mp);
        eina_mempool_statistics(mp);
     }

   eina_mempool_del(mp);
}

#ifdef EINA_BUILD_CHAINED_POOL
EFL_START_TEST(eina_mempool_chained_mempool)
{
   Eina_Mempool *mp;

   mp = eina_mempool_add("chained_mempool", "test", NULL, sizeof (int), 256);
   _eina_mempool_test(mp, EINA_FALSE, EINA_FALSE, EINA_TRUE);
}
EFL_END_TEST
#endif

#ifdef EINA_BUILD_PASS_THROUGH
EFL_START_TEST(eina_mempool_pass_through)
{
   Eina_Mempool *mp;

   mp = eina_mempool_add("pass_through", "test", NULL, sizeof (int), 8, 0);
   _eina_mempool_test(mp, EINA_TRUE, EINA_FALSE, EINA_FALSE);
}
EFL_END_TEST
#endif

void
eina_test_mempool(TCase *tc)
{
#ifdef EINA_BUILD_CHAINED_POOL
   tcase_add_test(tc, eina_mempool_chained_mempool);
#endif
#ifdef EINA_BUILD_PASS_THROUGH
   tcase_add_test(tc, eina_mempool_pass_through);
#endif
}
