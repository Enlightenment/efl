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

typedef struct _Test_Data
{
   Eina_Bool success_flag;
   unsigned int child_count;
} Test_Data;

typedef struct _Test_Child_Data
{
   Test_Data *tdata;
   unsigned int idx;
} Test_Child_Data;

const int child_number = 3;
const int base_ints[] = { 41, 42, 43 };

static void
_future_error_then(void *data EINA_UNUSED, Efl_Event const* event EINA_UNUSED)
{
   fprintf(stderr, "Promise failed!\n"); fflush(stderr);
   ck_abort_msg("Promise failed");
}

static void
_property_get_then(void *data, Efl_Event const *event)
{
   Test_Child_Data *t = data;
   Eina_Accessor *value_itt = (Eina_Accessor*)((Efl_Future_Event_Success*)event->info)->value;
   int v_int = 0;
   Eina_Bool v_bool = EINA_FALSE;

#define _value_get_and_compare(it, i, var, cmp)  \
   do { \
      Eina_Value *vvv = NULL; \
      if (!eina_accessor_data_get(it, i, (void **)&vvv) || !vvv || \
          !eina_value_get(vvv, &var) || var != cmp) \
        { \
           fprintf(stderr, "Could not get value!\n"); fflush(stderr); \
           ck_abort_msg("Could not get value"); \
           return; \
        } \
   } while(0)

   _value_get_and_compare(value_itt, 0, v_int, base_ints[t->idx]);
   _value_get_and_compare(value_itt, 1, v_bool, EINA_TRUE);
   _value_get_and_compare(value_itt, 2, v_bool, EINA_FALSE);

   t->tdata->child_count++;
   if (t->tdata->child_count == 3)
     t->tdata->success_flag = EINA_TRUE;

#undef _value_get_and_compare
}

static void
_children_slice_get_then(void *data, Efl_Event const* event)
{
   Eina_Accessor *children = (Eina_Accessor *)((Efl_Future_Event_Success*)event->info)->value;
   Efl_Model *child;
   Test_Child_Data *t;
   unsigned int i = 0;

   fprintf(stderr, "OPAAAAAAa\n");
   EINA_ACCESSOR_FOREACH(children, i, child)
     {
        Efl_Future *futures[3] = {NULL,};
        Efl_Future *future_all = NULL;

        futures[0] = efl_model_property_get(child, "test_p_int");
        futures[1] = efl_model_property_get(child, "test_p_true");
        futures[2] = efl_model_property_get(child, "test_p_false");

        future_all = efl_future_all(futures[0], futures[1], futures[2]);

        t = calloc(1, sizeof(Test_Child_Data));
        t->tdata = data;
        t->idx = i;
        efl_future_then(future_all, _property_get_then, _future_error_then, NULL, t);
     }
}

START_TEST(efl_test_model_composite_boolean)
{
   Efl_Model_Item *base_model, *child;
   int i;
   Eina_Value v;
   Efl_Model_Composite_Boolean *model;
   Test_Data *tdata;
   Efl_Future *future;

   fail_if(!ecore_init(), "ERROR: Cannot init Ecore!\n");
   fail_if(!efl_object_init(), "ERROR: Cannot init EO!\n");

   eina_value_setup(&v, EINA_VALUE_TYPE_INT);

   base_model = efl_add(EFL_MODEL_ITEM_CLASS, NULL);
   ck_assert(!!base_model);

   for (i = 0; i < child_number; ++i)
     {
        child = efl_model_child_add(base_model);
        ck_assert(!!child);
        ck_assert(eina_value_set(&v, base_ints[i]));
        efl_model_property_set(child, "test_p_int", &v);
     }

   model = efl_add(EFL_MODEL_COMPOSITE_BOOLEAN_CLASS, NULL,
                  efl_model_composite_boolean_composite_model_set(efl_added, base_model),
                  efl_model_composite_boolean_property_add(efl_added, "test_p_true", EINA_TRUE),
                  efl_model_composite_boolean_property_add(efl_added, "test_p_false", EINA_FALSE));
   ck_assert(!!model);

   tdata = calloc(1, sizeof(Test_Data));
   future = efl_model_children_slice_get(model, 0, 0);
   efl_future_then(future, _children_slice_get_then, _future_error_then, NULL, tdata);

   ecore_main_loop_iterate();

   ck_assert(tdata->success_flag);

   ecore_shutdown();
}
END_TEST

void
efl_test_case_model_composite_boolean(TCase *tc)
{
   tcase_add_test(tc, efl_test_model_composite_boolean);
}
