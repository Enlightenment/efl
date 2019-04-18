/* EFL - EFL library
 * Copyright (C) 2013 Cedric Bail
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

#include "efl_suite.h"

#include <Efl.h>
#include <Ecore.h>

typedef struct _Test_Container_Item_Data {
   unsigned int index;
} Test_Container_Item_Data;

const int base_int[7]  = {10, 11, 12, 13, 14, 0, 16};
const char * const base_str[7] = {"A", "B", "C", "D", "E", "", "GH"};

static Eina_Value
_children_slice_future_then(void *data EINA_UNUSED,
                            const Eina_Value v, const Eina_Future *dead_future EINA_UNUSED)
{
   unsigned int i, len;
   Efl_Model *child = NULL;

   fail_if(eina_value_type_get(&v) != EINA_VALUE_TYPE_ARRAY);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Eina_Value *value_int = NULL;
        Eina_Value *value_str = NULL;
        const char *cmp_str = NULL;
        int cmp_int = 0;

        value_int = efl_model_property_get(child, "test_p_int");
        value_str = efl_model_property_get(child, "test_p_str");

        fail_if(eina_value_type_get(value_int) != EINA_VALUE_TYPE_INT);
        fail_if(eina_value_type_get(value_str) != EINA_VALUE_TYPE_STRING);

        eina_value_get(value_int, &cmp_int);
        eina_value_get(value_str, &cmp_str);

         if (cmp_int != base_int[i] ||
             strcmp(cmp_str, base_str[i]) != 0)
           {
              abort();
           }
     }
   fprintf(stderr, "len: %i\n", len);

   fail_if(len != 7);

   ecore_main_loop_quit();

   return v;
}

EFL_START_TEST(efl_test_container_model_values)
{
   Efl_Container_Model* model;
   Eina_Future *future;
   int **cmp_int;
   const char **cmp_str;
   int i;

   cmp_int = calloc(8, sizeof(int*));
   cmp_str = calloc(8, sizeof(const char*));
   for (i = 0; i < 7; ++i)
     {
        cmp_int[i] = calloc(1, sizeof(int));
        *(cmp_int[i]) = base_int[i];
        cmp_str[i] = strdup(base_str[i]);
     }

   model = efl_add(EFL_CONTAINER_MODEL_CLASS, efl_main_loop_get());

   efl_container_model_child_property_add(model, "test_p_int", EINA_VALUE_TYPE_INT,
                                          eina_carray_iterator_new((void**)cmp_int));

   efl_container_model_child_property_add(model, "test_p_str", EINA_VALUE_TYPE_STRING,
                                          eina_carray_iterator_new((void**)cmp_str));

   for (i = 0; i < 7; ++i)
     {
        free(cmp_int[i]);
        free((void*)cmp_str[i]);
     }
   free(cmp_int);
   free(cmp_str);

   future = efl_model_children_slice_get(model, 0, efl_model_children_count_get(model));

   eina_future_then(future, _children_slice_future_then, NULL, NULL);

   ecore_main_loop_begin();

   efl_del(model);
}
EFL_END_TEST


void
efl_test_case_container_model(TCase *tc)
{
   tcase_add_test(tc, efl_test_container_model_values);
}
