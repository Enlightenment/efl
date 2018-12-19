/* EFL - EFL library
 * Copyright (C) 2018 Cedric Bail
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
static const char *_efl_test_model_view_label_format = "Index %i.";
static const char *dependences[] = { "test_p_int" };

static Eina_Value *
_efl_test_model_view_label_get(void *data, const Efl_Model_View *mv, Eina_Stringshare *property)
{
   Eina_Strbuf *buf;
   Eina_Value *r;
   Eina_Value *p_int;
   int v_int;
   const char *format = data;

   ck_assert(strcmp(property, "label") == 0);

   p_int = efl_model_property_get(mv, "test_p_int");
   ck_assert(eina_value_int_get(p_int, &v_int) == EINA_TRUE);

   buf = eina_strbuf_new();
   eina_strbuf_append_printf(buf, format, v_int);

   r = eina_value_string_new(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);

   return r;
}

static Eina_Future *
_efl_test_model_view_label_set(void *data EINA_UNUSED, Efl_Model_View *mv, Eina_Stringshare *property EINA_UNUSED, Eina_Value *value EINA_UNUSED)
{
   return efl_loop_future_rejected(mv, EFL_MODEL_ERROR_READ_ONLY);
}

static void
_efl_test_model_view_label_clean(void *data)
{
   ck_assert_ptr_eq(data, _efl_test_model_view_label_format);
}

static Eina_Value *
_efl_test_model_view_color_get(void *data EINA_UNUSED, const Efl_Model_View *mv, Eina_Stringshare *property)
{
   Eina_Strbuf *buf;
   Eina_Value *r;
   Eina_Value *p_int;
   int v_int;

   ck_assert(strcmp(property, "color") == 0);

   p_int = efl_model_property_get(mv, "test_p_int");
   ck_assert(eina_value_int_get(p_int, &v_int) == EINA_TRUE);

   buf = eina_strbuf_new();
   eina_strbuf_append_printf(buf, "#%02x%02x%02x%02x", v_int, v_int, v_int, v_int);

   r = eina_value_string_new(eina_strbuf_string_get(buf));
   eina_strbuf_free(buf);

   return r;
}

static Eina_Future *
_efl_test_model_view_color_set(void *data EINA_UNUSED, Efl_Model_View *mv, Eina_Stringshare *property EINA_UNUSED, Eina_Value *value EINA_UNUSED)
{
   return efl_loop_future_rejected(mv, EFL_MODEL_ERROR_READ_ONLY);
}

static void
_efl_test_model_view_color_clean(void *data EINA_UNUSED)
{
}

static Eina_Value
_dummy(Eo *obj EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value value)
{
   ck_abort();

   return value;
}

static Eina_Value
_expected_fail(Eo *o EINA_UNUSED, void *data EINA_UNUSED, Eina_Error error)
{
   ck_assert_int_eq(error, EFL_MODEL_ERROR_READ_ONLY);
   return eina_value_int_init(0);
}

static unsigned int counting = 0;

typedef struct _Property_Change_Test Property_Change_Test;
struct _Property_Change_Test
{
   Eina_Bool checked;
   const char *name;
};

static void
_properties_changed(void *data, const Efl_Event *event)
{
   Efl_Model_Property_Event *ev = event->info;
   unsigned int *count = data;
   const char *property;
   Eina_Array_Iterator iterator;
   unsigned int i, j;
   Property_Change_Test tests[] = {
      { EINA_FALSE, "test_p_int" },
      { EINA_FALSE, "color" },
      { EINA_FALSE, "label" }
   };

   EINA_ARRAY_ITER_NEXT(ev->changed_properties, i, property, iterator)
     {
        for (j = 0; j < EINA_C_ARRAY_LENGTH(tests); j++)
          if (!strcmp(tests[j].name, property))
            {
               fail_if(tests[j].checked);
               tests[j].checked = EINA_TRUE;
               break;
            }
     }

   for (j = 0; j < EINA_C_ARRAY_LENGTH(tests); j++)
     {
        fail_if(!tests[j].checked);
     }

   *count += 1;
}

static Eina_Value
_efl_test_model_view_child_get(Eo *obj EINA_UNUSED,
                               void *data EINA_UNUSED,
                               const Eina_Value v)
{
   Efl_Model *child;
   Eina_Future **all = NULL;
   Eina_Future *r;
   unsigned int i, len;

   fail_if(eina_value_type_get(&v) != EINA_VALUE_TYPE_ARRAY);

   all = calloc(eina_value_array_count(&v) + 1, sizeof (Eina_Future*));
   if (!all) return eina_value_error_init(ENOMEM);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Eina_Value *p_int = NULL;
        Eina_Value *p_color = NULL;
        Eina_Value *p_label = NULL;
        Eina_Value *p_deadend = NULL;
        Eina_Error err = 0;
        int v_int = 0;

        p_int = efl_model_property_get(child, "test_p_int");
        ck_assert(eina_value_int_get(p_int, &v_int));

        p_color = efl_model_property_get(child, "color");
        p_label = efl_model_property_get(child, "label");
        p_deadend = efl_model_property_get(child, "deadend");

        ck_assert_ptr_eq(eina_value_type_get(p_deadend), EINA_VALUE_TYPE_ERROR);
        eina_value_error_get(p_deadend, &err);
        ck_assert_int_eq(err, EFL_MODEL_ERROR_NOT_SUPPORTED);

        efl_event_callback_add(child, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _properties_changed, &counting);

        efl_future_then(child, efl_model_property_set(child, "color", p_label),
                        .success = _dummy, .error = _expected_fail);
        efl_future_then(child, efl_model_property_set(child, "color", p_color),
                        .success = _dummy, .error = _expected_fail);
        efl_future_then(child, efl_model_property_set(child, "deadend", eina_value_int_new(42)),
                        .success = _dummy, .error = _expected_fail);

        all[i] = efl_model_property_set(child, "test_p_int", eina_value_int_new(v_int+100));

        eina_value_free(p_color);
        eina_value_free(p_label);
        eina_value_free(p_deadend);
     }

   all[i] = EINA_FUTURE_SENTINEL;
   r = eina_future_all_array(all);
   return eina_future_as_value(r);
}

static Eina_Value
_efl_test_model_view_child_fetch(Eo *mv,
                                 void *data EINA_UNUSED,
                                 const Eina_Value v)
{
   Eina_Future *r;

   r = efl_model_children_slice_get(mv, 0, efl_model_children_count_get(mv));
   return eina_future_as_value(r);
}

static Eina_Value
_efl_test_model_view_child_updated_get(Eo *obj EINA_UNUSED,
                                       void *data EINA_UNUSED,
                                       const Eina_Value v)
{
   Efl_Model *child;
   unsigned int i, len;

   fail_if(eina_value_type_get(&v) != EINA_VALUE_TYPE_ARRAY);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Eina_Value *p_int = NULL;
        Eina_Value *p_color = NULL;
        Eina_Value *p_label = NULL;
        const char *s_color = NULL;
        const char *s_label = NULL;
        int v_int = 0;

        p_int = efl_model_property_get(child, "test_p_int");
        ck_assert(eina_value_int_get(p_int, &v_int));
        ck_assert_int_gt(v_int, 100);

        p_color = efl_model_property_get(child, "color");
        p_label = efl_model_property_get(child, "label");
        eina_value_string_get(p_color, &s_color);
        eina_value_string_get(p_label, &s_label);

        ck_assert_ptr_ne(s_color, NULL);
        ck_assert_ptr_ne(s_label, NULL);
     }

   return eina_value_int_init(0);
}

static Eina_Value
_efl_test_model_view_tests_end(Eo *obj,
                               void *data EINA_UNUSED,
                               const Eina_Value v)
{
   efl_loop_quit(efl_loop_get(obj), v);
   return v;
}

EFL_START_TEST(efl_test_model_view)
{
   Efl_Model_Item *base_model, *child, *mv;
   Eina_Future *f;
   int i;
   Eina_Value v;

   base_model = efl_add_ref(EFL_MODEL_ITEM_CLASS, efl_main_loop_get());
   ck_assert(!!base_model);

   for (i = 0; i < child_number; ++i)
     {
        child = efl_model_child_add(base_model);
        ck_assert(!!child);
        v = eina_value_int_init(base_ints[i]);
        efl_model_property_set(child, "test_p_int", &v);
     }

   mv = efl_add_ref(EFL_MODEL_VIEW_CLASS, efl_main_loop_get(),
                    efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!mv);

   efl_model_view_property_logic_add(mv, "label",
                                     (void*) _efl_test_model_view_label_format, _efl_test_model_view_label_get, _efl_test_model_view_label_clean,
                                     (void*) _efl_test_model_view_label_format, _efl_test_model_view_label_set, _efl_test_model_view_label_clean,
                                     EINA_C_ARRAY_ITERATOR_NEW(dependences));

   efl_model_view_property_logic_add(mv, "color",
                                     NULL, _efl_test_model_view_color_get, _efl_test_model_view_color_clean,
                                     NULL, _efl_test_model_view_color_set, _efl_test_model_view_color_clean,
                                     EINA_C_ARRAY_ITERATOR_NEW(dependences));

   efl_model_view_property_logic_add(mv, "deadend",
                                     NULL, NULL, NULL,
                                     NULL, NULL, NULL,
                                     NULL);

   f = efl_model_children_slice_get(mv, 0, efl_model_children_count_get(mv));
   f = efl_future_then(mv, f, .success_type = EINA_VALUE_TYPE_ARRAY,
                       .success = _efl_test_model_view_child_get);
   f = efl_future_then(mv, f, .success = _efl_test_model_view_child_fetch);
   f = efl_future_then(mv, f, .success_type = EINA_VALUE_TYPE_ARRAY,
                       .success = _efl_test_model_view_child_updated_get);
   f = efl_future_then(mv, f, .success_type = EINA_VALUE_TYPE_INT,
                       .success = _efl_test_model_view_tests_end);

   // And run !
   ecore_main_loop_begin();

   // We expect each child to have their property updated
   ck_assert_int_eq(counting, child_number);
}
EFL_END_TEST

void
efl_test_case_model_view(TCase *tc)
{
   tcase_add_test(tc, efl_test_model_view);
}
