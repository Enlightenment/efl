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

#include <stdlib.h>
#include <time.h>

#include <stdio.h>

#include "eina_bench.h"
#include "eina_array.h"
#include "eina_list.h"
#include "eina_inlist.h"

typedef struct _Eina_Bench_Object Eina_Bench_Object;
struct _Eina_Bench_Object
{
   Eina_Inlist __list;

   void *somewhere;
   int data;
   Eina_Bool keep;
};

static Eina_Bool keep(void *data, __UNUSED__ void *gdata)
{
   Eina_Bench_Object *bo = data;

   if (bo->keep == EINA_TRUE) return EINA_TRUE;
   free(bo);
   return EINA_FALSE;
}

static void
eina_bench_array_4evas_render_inline(int request)
{
   Eina_Array *array;
   Eina_Bench_Object *ebo;
   unsigned int i;
   unsigned int j;

   srand(time(NULL));

   eina_array_init();

   array = eina_array_new(64);

   for (i = 0; i < 1000; ++i)
     {
	for (j = 0; j < (unsigned int) request; ++j)
	  {
	     ebo = malloc(sizeof (Eina_Bench_Object));
	     if (!ebo) continue ;

	     ebo->keep = rand() < (RAND_MAX / 2) ? EINA_TRUE : EINA_FALSE;

	     eina_array_append(array, ebo);
	  }

	if (i == 500)
	  {
	     EINA_ARRAY_ITER_NEXT(array, j, ebo)
	       free(ebo);
	     EINA_ARRAY_ITER_END;

	     eina_array_clean(array);
	  }
	else if (i % 30 == 0) eina_array_remove(array, keep, NULL);

	EINA_ARRAY_ITER_NEXT(array, j, ebo)
	  ebo->keep = rand() < (RAND_MAX / 2) ? ebo->keep : EINA_FALSE;
	EINA_ARRAY_ITER_END;
     }

   EINA_ARRAY_ITER_NEXT(array, j, ebo)
     free(ebo);
   EINA_ARRAY_ITER_END;

   eina_array_free(array);

   eina_array_shutdown();
}

static void
eina_bench_list_4evas_render(int request)
{
   Eina_List *list = NULL;
   Eina_List *tmp;
   Eina_Bench_Object *ebo;
   int i;
   int j;

   eina_list_init();

   for (i = 0; i < 1000; ++i)
     {
	for (j = 0; j < request; ++j)
	  {
	     ebo = malloc(sizeof (Eina_Bench_Object));
	     if (!ebo) continue ;

	     ebo->keep = rand() < (RAND_MAX / 2) ? EINA_TRUE : EINA_FALSE;

	     list = eina_list_prepend(list, ebo);
	  }

	if (i == 500)
	  {
	     while (list)
	       {
		  free(eina_list_data(list));
		  list = eina_list_remove_list(list, list);
	       }
	  }
	else
	  {
	     if (i % 30 == 0)
	       {
		  tmp = list;
		  while (tmp)
		    {
		       Eina_List *reminder = tmp;

		       ebo = eina_list_data(reminder);
		       tmp = eina_list_next(tmp);

		       if (ebo->keep == EINA_FALSE)
			 {
			    list = eina_list_remove_list(list, reminder);
			    free(ebo);
			 }
		    }
	       }
	  }

	for (tmp = list; tmp; tmp = eina_list_next(tmp))
	  {
	     ebo = eina_list_data(tmp);

	     ebo->keep = rand() < (RAND_MAX / 2) ? ebo->keep : EINA_FALSE;
	  }
     }

   while (list)
     {
	free(eina_list_data(list));
	list = eina_list_remove_list(list, list);
     }

   eina_list_shutdown();
}

static void
eina_bench_inlist_4evas_render(int request)
{
   Eina_Inlist *head = NULL;
   Eina_Inlist *tmp;
   Eina_Bench_Object *ebo;
   int i;
   int j;

   for (i = 0; i < 1000; ++i)
     {
	for (j = 0; j < request; ++j)
	  {
	     ebo = malloc(sizeof (Eina_Bench_Object));
	     if (!ebo) continue ;

	     ebo->keep = rand() < (RAND_MAX / 2) ? EINA_TRUE : EINA_FALSE;

	     head = eina_inlist_prepend(head, ebo);
	  }

	if (i == 500)
	  {
	     while (head)
	       {
		  tmp = head;
		  head = head->next;
		  free(tmp);
	       }
	  }
	else
	  {
	     if (i % 30 == 0)
	       {
		  tmp = head;
		  while(tmp)
		    {
		       ebo = (Eina_Bench_Object *) tmp;

		       tmp = tmp->next;
		       if (ebo->keep == EINA_FALSE)
			 {
			    head = eina_inlist_remove(head, ebo);
			    free(ebo);
			 }
		    }
	       }
	  }

	EINA_INLIST_ITER_NEXT(head, ebo)
	  ebo->keep = rand() < (RAND_MAX / 2) ? ebo->keep : EINA_FALSE;
     }

   while (head)
     {
	tmp = head;
	head = head->next;
	free(tmp);
     }
}

void eina_bench_array(Eina_Bench *bench)
{
   eina_bench_register(bench, "array-inline", EINA_BENCH(eina_bench_array_4evas_render_inline), 200, 4000, 100);
   eina_bench_register(bench, "list", EINA_BENCH(eina_bench_list_4evas_render), 200, 4000, 100);
   eina_bench_register(bench, "inlist", EINA_BENCH(eina_bench_inlist_4evas_render), 200, 4000, 100);
}

