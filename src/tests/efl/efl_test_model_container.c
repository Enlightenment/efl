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

#include <Ecore.h>
#include <Efl.h>

typedef struct _Test_Container_Data {
   int item_count;
   Eina_Bool pass_flag;
   Eina_Bool fail_flag;
} Test_Container_Data;

typedef struct _Test_Container_Item_Data {
   Test_Container_Data* test_data;
   unsigned int index;
} Test_Container_Item_Data;

const int base_int[7]  = {10, 11, 12, 13, 14, 0, 16};
const char * const base_str[7] = {"A", "B", "C", "D", "E", "", "GH"};

static void
_container_property_get_then(void *data, void *values)
{
   Eina_Iterator *value_itt = values;
   Test_Container_Item_Data *test_item_data = data;
   Eina_Value *value_int = NULL;
   Eina_Value *value_str = NULL;
   int cmp_int = 0;
   const char *cmp_str = NULL;

   test_item_data->test_data->item_count++;

   if (!eina_iterator_next(value_itt, (void**)&value_int) ||
       !eina_iterator_next(value_itt, (void**)&value_str))
     {
        test_item_data->test_data->fail_flag = EINA_TRUE;
        return;
     }

   eina_value_get(value_int, &cmp_int);
   eina_value_get(value_str, &cmp_str);

   if (cmp_int != base_int[test_item_data->index] ||
       strcmp(cmp_str, base_str[test_item_data->index]) != 0)
     {
        test_item_data->test_data->fail_flag = EINA_TRUE;
     }

   if (test_item_data->test_data->item_count == 7)
     {
        test_item_data->test_data->pass_flag = EINA_TRUE;
     }
}

static void
_children_slice_promise_then(void *data, void *value)
{
   Eina_Accessor *children_accessor = value;
   unsigned int i = 0;
   Efl_Model *child;

   if (children_accessor)
     {
        EINA_ACCESSOR_FOREACH(children_accessor, i, child)
          {
             Eina_Promise *promises[3] = {NULL,};
             Eina_Promise *promise_all = NULL;
             Test_Container_Item_Data *test_item_data = calloc(1, sizeof(Test_Container_Item_Data));

             test_item_data->test_data = data;
             test_item_data->index = i;

             promises[0] = efl_model_property_get(child, "test_p_int");
             promises[1] = efl_model_property_get(child, "test_p_str");

             promise_all = eina_promise_all(eina_carray_iterator_new((void**)promises));
             eina_promise_then(promise_all, _container_property_get_then, NULL, test_item_data);
          }
     }
}


START_TEST(efl_test_model_container_values)
{
   Efl_Model_Container* model;
   Eina_Promise *promise;
   Test_Container_Data test_data;
   int **cmp_int;
   const char **cmp_str;
   int i;

   eo_init();

   cmp_int = calloc(8, sizeof(int*));
   cmp_str = calloc(8, sizeof(const char*));
   for (i = 0; i < 7; ++i)
     {
        cmp_int[i] = calloc(1, sizeof(int));
        *(cmp_int[i]) = base_int[i];
        cmp_str[i] = strdup(base_str[i]);
     }

   model = eo_add(EFL_MODEL_CONTAINER_CLASS, NULL);

   efl_model_container_child_property_add(model, "test_p_int", EINA_VALUE_TYPE_INT,
                                          eina_carray_iterator_new((void**)cmp_int));
   efl_model_container_child_property_add(model, "test_p_str", EINA_VALUE_TYPE_STRING,
                                          eina_carray_iterator_new((void**)cmp_str));

   for (i = 0; i < 7; ++i)
     {
        free(cmp_int[i]);
        free(cmp_str[i]);
     }
   free(cmp_int);
   free(cmp_str);

   promise = efl_model_children_slice_get(model, 0, 0);

   test_data.item_count = 0;
   test_data.pass_flag = EINA_FALSE;
   test_data.fail_flag = EINA_FALSE;

   eina_promise_then(promise, _children_slice_promise_then, NULL, &test_data);

   ecore_main_loop_iterate();

   ck_assert(!!test_data.pass_flag);
   ck_assert(!test_data.fail_flag);

   eo_shutdown();
}
END_TEST

void
efl_test_case_model_container(TCase *tc)
{
   tcase_add_test(tc, efl_test_model_container_values);
}
