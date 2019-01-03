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

static const int child_number = 3;
static const int base_ints[] = { 41, 42, 43 };
static const Eina_Bool base_selections[] = { EINA_FALSE, EINA_FALSE, EINA_TRUE };

static Eina_Value
_children_slice_get_then(void *data EINA_UNUSED,
                         const Eina_Value v,
                         const Eina_Future *dead_future EINA_UNUSED)
{
   unsigned int i, len;
   Efl_Model *child;

   fail_if(eina_value_type_get(&v) != EINA_VALUE_TYPE_ARRAY);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Eina_Value *p_int = NULL;
        Eina_Value *p_true = NULL;
        Eina_Value *p_false = NULL;
        int v_int = 0;
        Eina_Bool v_true = EINA_FALSE;
        Eina_Bool v_false = EINA_TRUE;

        p_int = efl_model_property_get(child, "test_p_int");
        p_true = efl_model_property_get(child, "test_p_true");
        p_false = efl_model_property_get(child, "test_p_false");

        eina_value_get(p_int, &v_int);
        eina_value_get(p_true, &v_true);
        eina_value_get(p_false, &v_false);

        fail_if(v_int != base_ints[i]);
        fail_if(v_true != EINA_TRUE);
        fail_if(v_false != EINA_FALSE);
     }

   ecore_main_loop_quit();

   return v;
}

static Eina_Value
_selection_children_slice_get_then(void *data EINA_UNUSED,
                                   const Eina_Value v,
                                   const Eina_Future *dead_future EINA_UNUSED)
{
   unsigned int i, len;
   Efl_Model *child = NULL;

   fail_if(eina_value_type_get(&v) != EINA_VALUE_TYPE_ARRAY);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Eina_Value *p_int = NULL;
        Eina_Value *p_bool = NULL;
        Eina_Value *p_index = NULL;
        int v_int = 0;
        unsigned int index = 0;
        Eina_Bool v_bool = EINA_FALSE;

        p_bool = efl_model_property_get(child, "selected");
        p_int = efl_model_property_get(child, "test_p_int");
        p_index = efl_model_property_get(child, "child.index");

        eina_value_get(p_bool, &v_bool);
        eina_value_get(p_int, &v_int);
        fail_if(!eina_value_uint_convert(p_index, &index));

        fail_if(v_bool != base_selections[i]);
        fail_if(v_int != base_ints[i]);
        ck_assert_int_eq(i, index);
     }

   ecore_main_loop_quit();

   return v;
}

EFL_START_TEST(efl_test_model_composite_boolean)
{
   Efl_Model_Item *base_model, *child;
   int i;
   Eina_Value v;
   Efl_Model_Composite_Boolean *model;
   Eina_Future *future;

   eina_value_setup(&v, EINA_VALUE_TYPE_INT);

   base_model = efl_add_ref(EFL_MODEL_ITEM_CLASS, efl_main_loop_get());
   ck_assert(!!base_model);

   for (i = 0; i < child_number; ++i)
     {
        child = efl_model_child_add(base_model);
        ck_assert(!!child);
        ck_assert(eina_value_set(&v, base_ints[i]));
        efl_model_property_set(child, "test_p_int", &v);
     }

   model = efl_add_ref(EFL_MODEL_COMPOSITE_BOOLEAN_CLASS, efl_main_loop_get(),
                  efl_ui_view_model_set(efl_added, base_model),
                  efl_model_composite_boolean_add(efl_added, "test_p_true", EINA_TRUE),
                  efl_model_composite_boolean_add(efl_added, "test_p_false", EINA_FALSE));
   ck_assert(!!model);

   future = efl_model_children_slice_get(model, 0, efl_model_children_count_get(model));
   eina_future_then(future, _children_slice_get_then, NULL, NULL);

   ecore_main_loop_begin();
}
EFL_END_TEST

static Eina_Value
_wait_propagate(void *data EINA_UNUSED,
                const Eina_Value v,
                const Eina_Future *dead_future EINA_UNUSED)
{
   ecore_main_loop_quit();
   return v;
}

EFL_START_TEST(efl_test_model_composite_selection)
{
   Efl_Model_Item *base_model, *child;
   int i;
   Eina_Value v;
   Efl_Model_Composite_Selection *model;
   Eina_Future *future;

   eina_value_setup(&v, EINA_VALUE_TYPE_INT);

   base_model = efl_add_ref(EFL_MODEL_ITEM_CLASS, efl_main_loop_get());
   ck_assert(!!base_model);

   for (i = 0; i < child_number; ++i)
     {
        child = efl_model_child_add(base_model);
        ck_assert(!!child);
        ck_assert(eina_value_set(&v, base_ints[i]));
        efl_model_property_set(child, "test_p_int", &v);
     }

   model = efl_add_ref(EFL_MODEL_COMPOSITE_SELECTION_CLASS, efl_main_loop_get(),
                   efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);
   future = efl_model_property_set(model, "child.selected", eina_value_int_new(2));
   eina_future_then(future, _wait_propagate, NULL, NULL);
   ecore_main_loop_begin();

   future = efl_model_children_slice_get(model, 0, efl_model_children_count_get(model));
   eina_future_then(future, _selection_children_slice_get_then, NULL, NULL);

   ecore_main_loop_begin();
}
EFL_END_TEST

void
efl_test_case_model_composite_boolean(TCase *tc)
{
   tcase_add_test(tc, efl_test_model_composite_boolean);
   tcase_add_test(tc, efl_test_model_composite_selection);
}
