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

#include <limits.h>

#include "efl_suite.h"

#include <Efl.h>
#include <Ecore.h>

static const int child_number = 7;
static const int base_ints[] = { 41, 42, 43, 44, 45, 46, 47 };

static void
_cleanup(Eo *o EINA_UNUSED, void *data EINA_UNUSED, const Eina_Future *dead_future EINA_UNUSED)
{
   ecore_main_loop_quit();
}

static Eina_Value
_children_slice_get_then(Eo *o EINA_UNUSED,
                         void *data EINA_UNUSED,
                         const Eina_Value v)
{
   unsigned int i, len;
   Efl_Model *child;
   Eina_Future **all;

   all = alloca(sizeof (Eina_Future *) * (eina_value_array_count(&v) + 1));

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Eina_Value *p_int = NULL;
        Eina_Value *p_true = NULL;
        Eina_Value *p_false = NULL;
        int v_int = -1;
        Eina_Bool v_true = EINA_FALSE;
        Eina_Bool v_false = EINA_TRUE;

        p_int = efl_model_property_get(child, "test_p_int");
        p_true = efl_model_property_get(child, "test_p_true");
        p_false = efl_model_property_get(child, "test_p_false");

        fail_if(!eina_value_int_get(p_int, &v_int));
        fail_if(!eina_value_bool_get(p_true, &v_true));
        fail_if(!eina_value_bool_get(p_false, &v_false));

        all[i] = efl_model_property_set(child, "test_odd_even", v_int & 0x1 ? p_true : p_false);

        fail_if(v_int != base_ints[i]);
        fail_if(v_true != EINA_TRUE);
        fail_if(v_false != EINA_FALSE);

        eina_value_free(p_int);
        eina_value_free(p_true);
        eina_value_free(p_false);
     }
   all[i] = EINA_FUTURE_SENTINEL;

   return eina_future_as_value(eina_future_all_array(all));
}

static Eina_Value
_children_odd_even_ready(Eo *model, void *data EINA_UNUSED, const Eina_Value v EINA_UNUSED)
{
   return eina_future_as_value(efl_model_children_slice_get(model, 0, child_number));
}

static Eina_Value
_children_odd_even_check(Eo *model, void *data EINA_UNUSED, const Eina_Value v)
{
   unsigned int i, len;
   Efl_Model *child, *first_child = NULL;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Eina_Value *p_odd_even = NULL;
        Eina_Value *p_int = NULL;
        Eina_Bool v_odd_even = EINA_TRUE;
        int v_int = -1;

        if (!first_child) first_child = child;

        p_int = efl_model_property_get(child, "test_p_int");
        p_odd_even = efl_model_property_get(child, "test_odd_even");

        fail_if(!eina_value_bool_get(p_odd_even, &v_odd_even));
        fail_if(!eina_value_int_get(p_int, &v_int));

        fail_if(v_int & 0x1 ? !v_odd_even : v_odd_even);

        eina_value_free(p_int);
        eina_value_free(p_odd_even);
     }

   efl_model_child_del(model, first_child);

   return eina_future_as_value(efl_model_children_slice_get(model, 0, child_number - 1));
}

static Eina_Value
_children_odd_even_check_after_removal(Eo *model EINA_UNUSED, void *data EINA_UNUSED,
                                       const Eina_Value v)
{
   unsigned int i, len;
   Efl_Model *child;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Eina_Value *p_odd_even = NULL;
        Eina_Value *p_int = NULL;
        Eina_Bool v_odd_even = EINA_TRUE;
        int v_int = -1;

        p_int = efl_model_property_get(child, "test_p_int");
        p_odd_even = efl_model_property_get(child, "test_odd_even");

        fail_if(!eina_value_bool_get(p_odd_even, &v_odd_even));
        fail_if(!eina_value_int_get(p_int, &v_int));

        fail_if(v_int & 0x1 ? !v_odd_even : v_odd_even);
        fail_if(v_int != base_ints[i + 1]);

        eina_value_free(p_int);
        eina_value_free(p_odd_even);
     }

   return v;
}

static Eina_Value
_assert_on_error(Eo *o EINA_UNUSED, void *data EINA_UNUSED, Eina_Error error)
{
   ck_abort_msg("Assert on error triggered duing Boolean Model test with error '%s'.\n",
                eina_error_msg_get(error));
   return eina_value_error_init(error);
}

static unsigned int request[3] = { 3, 1, 5 };

static Eina_Value
_children_random_index(Eo *model EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value v)
{
   unsigned int i, len;
   Efl_Model *child;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        unsigned int index;

        index = efl_composite_model_index_get(child);
        ck_assert_int_eq(index, request[i]);
     }

   return v;
}

EFL_START_TEST(efl_test_boolean_model)
{
   Efl_Generic_Model *base_model, *child;
   int i;
   Eina_Value v = { 0 };
   Efl_Boolean_Model *model;
   Eina_Future *future;

   eina_value_setup(&v, EINA_VALUE_TYPE_INT);

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   ck_assert(!!base_model);

   for (i = 0; i < child_number; ++i)
     {
        child = efl_model_child_add(base_model);
        ck_assert(!!child);
        ck_assert(eina_value_set(&v, base_ints[i]));
        efl_model_property_set(child, "test_p_int", &v);
     }

   model = efl_add_ref(EFL_BOOLEAN_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model),
                       efl_boolean_model_boolean_add(efl_added, "test_p_true", EINA_TRUE),
                       efl_boolean_model_boolean_add(efl_added, "test_p_false", EINA_FALSE),
                       efl_boolean_model_boolean_add(efl_added, "test_odd_even", EINA_FALSE));
   ck_assert(!!model);

   future = efl_model_children_slice_get(model, 0, efl_model_children_count_get(model));
   future = efl_future_then(model, future,
                            .success = _children_slice_get_then,
                            .success_type = EINA_VALUE_TYPE_ARRAY);
   future = efl_future_then(model, future,
                            .success = _children_odd_even_ready,
                            .success_type = EINA_VALUE_TYPE_ARRAY);
   future = efl_future_then(model, future,
                            .success = _children_odd_even_check,
                            .success_type = EINA_VALUE_TYPE_ARRAY);
   future = efl_future_then(model, future,
                            .success = _children_odd_even_check_after_removal,
                            .success_type = EINA_VALUE_TYPE_ARRAY);
   future = efl_future_then(model, future, .error = _assert_on_error, .free = _cleanup);

   ecore_main_loop_begin();

   future = efl_model_children_index_get(model, EINA_C_ARRAY_ITERATOR_NEW(request));
   future = efl_future_then(model, future,
                            .success = _children_random_index,
                            .success_type = EINA_VALUE_TYPE_ARRAY);
   future = efl_future_then(model, future, .error = _assert_on_error, .free = _cleanup);

   ecore_main_loop_begin();
}
EFL_END_TEST

typedef struct _Efl_Filter_Model_Wait Efl_Filter_Model_Wait;
struct _Efl_Filter_Model_Wait
{
   Efl_Model *child;
   Efl_Model *parent;
   Eina_Promise *p;
};

static void _wait_for_it(void *data, const Efl_Event *event);

static void
_wait_cleanup(Efl_Filter_Model_Wait *mw)
{
   efl_event_callback_del(mw->child, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _wait_for_it, mw);
   efl_unref(mw->child);
   efl_unref(mw->parent);
   free(mw);
}

static void
_wait_for_it(void *data, const Efl_Event *event)
{
   Efl_Model_Property_Event *ev = event->info;
   Efl_Filter_Model_Wait *mw = data;
   const char *property;
   unsigned int i;
   Eina_Array_Iterator it;

   EINA_ARRAY_ITER_NEXT(ev->changed_properties, i, property, it)
     {
        if (!strcmp(property, "ready"))
          {
             Eina_Value *value = efl_model_property_get(event->object, "ready");
             Eina_Bool r;
             Eina_Error err = 0;

             if (eina_value_type_get(value) == EINA_VALUE_TYPE_BOOL &&
                 eina_value_bool_get(value, &r) &&
                 r)
               {
                  eina_value_free(value);
                  eina_promise_resolve(mw->p, eina_value_bool_init(EINA_TRUE));
                  _wait_cleanup(mw);
                  return ;
               }
             if (eina_value_type_get(value) == EINA_VALUE_TYPE_ERROR &&
                 eina_value_error_get(value, &err) &&
                 err != EAGAIN)
               {
                  eina_value_free(value);
                  eina_promise_reject(mw->p, err);
                  _wait_cleanup(mw);
                  return ;
               }
             eina_value_free(value);
             break;
          }
     }
}

static void
_wait_cancel(void *data, const Eina_Promise *dead_ptr EINA_UNUSED)
{
   _wait_cleanup(data);
}

static Eina_Future *
_wait_ready_true(Efl_Model *parent, Efl_Model *child)
{
   Efl_Filter_Model_Wait *mw;
   Eina_Value *value = efl_model_property_get(child, "ready");
   Eina_Bool r = EINA_FALSE;
   Eina_Error err = 0;

   if (eina_value_type_get(value) == EINA_VALUE_TYPE_BOOL &&
       eina_value_bool_get(value, &r) &&
       r)
     {
        eina_value_free(value);
        return efl_loop_future_resolved(parent, eina_value_bool_init(EINA_TRUE));
     }
   if (eina_value_type_get(value) == EINA_VALUE_TYPE_ERROR &&
       eina_value_error_get(value, &err) &&
       err != EAGAIN)
     {
        eina_value_free(value);
        return efl_loop_future_rejected(parent, err);
     }

   eina_value_free(value);

   mw = calloc(1, sizeof (Efl_Filter_Model_Wait));
   if (!mw) return efl_loop_future_rejected(parent, ENOMEM);

   mw->p = eina_promise_new(efl_loop_future_scheduler_get(parent), _wait_cancel, mw);
   mw->child = efl_ref(child);
   mw->parent = efl_ref(parent);

   efl_event_callback_add(child, EFL_MODEL_EVENT_PROPERTIES_CHANGED,
                          _wait_for_it, mw);

   return efl_future_then(child, eina_future_new(mw->p));
}

static Eina_Future *
_filter_cb(void *data EINA_UNUSED, Efl_Filter_Model *parent, Efl_Model *child)
{
   return _wait_ready_true(parent, child);
}

static Eina_Value
_fetch_and_switch(Eo *o EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value v)
{
   unsigned int i, len;
   Eo *target = NULL;
   Eina_Future **all;

   all = calloc(1, sizeof (Eina_Future *) * (eina_value_array_count(&v) + 1));
   if (!all) return eina_value_error_init(ENOMEM);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, target)
     {
        all[i] = efl_model_property_set(target, "ready", eina_value_bool_new(EINA_TRUE));
     }
   all[i] = EINA_FUTURE_SENTINEL;

   return eina_future_as_value(eina_future_all_array(all));
}

static void
_count_changed(void *data, const Efl_Event *ev)
{
   Eina_Promise *p = data;

   if (efl_model_children_count_get(ev->object) != (unsigned int)child_number) return ;

   efl_event_callback_del(ev->object, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED,
                          _count_changed, p);
   eina_promise_resolve(p, eina_value_bool_init(EINA_TRUE));
}

static void
_wait_count_cancel(void *data, const Eina_Promise *dead_ptr EINA_UNUSED)
{
   Eo *filter = data;

   efl_unref(filter);
}

static Eina_Value
_wait_count(Eo *filter, void *data EINA_UNUSED, const Eina_Value v EINA_UNUSED)
{
   Eina_Promise *p;
   Eina_Future *racing[3];
   Eina_Future *race;

   p = eina_promise_new(efl_loop_future_scheduler_get(filter),
                        _wait_count_cancel, efl_ref(filter));
   efl_event_callback_add(filter, EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _count_changed, p);

   racing[0] = eina_future_new(p);
   racing[1] = efl_loop_timeout(efl_loop_get(filter), 0.3);
   racing[2] = EINA_FUTURE_SENTINEL;

   race = eina_future_race_array(racing);

   return eina_future_as_value(race);
}

static Eina_Value
_count_and_fetch(Eo *filter, void *data EINA_UNUSED, const Eina_Value v EINA_UNUSED)
{
   ck_assert_int_eq(efl_model_children_count_get(filter), child_number);

   return eina_future_as_value(efl_model_children_slice_get(filter, 0, child_number));
}

static Eina_Value
_check_index(Eo *o EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value v)
{
   unsigned int i, len;
   Eo *target = NULL;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, target)
     {
        Eina_Value *p_original;
        uint64_t idx, original;

        idx = efl_composite_model_index_get(target);
        p_original = efl_model_property_get(target, "original");
        fail_if(!eina_value_uint64_convert(p_original, &original));
        ck_assert(original < (uint64_t)child_number);
        ck_assert(idx < (uint64_t)child_number);

        eina_value_free(p_original);
     }

   return v;
}

EFL_START_TEST(efl_test_filter_model)
{
   Efl_Generic_Model *base_model, *child;
   int i;
   Eina_Value v = { 0 };
   Efl_Boolean_Model *model;
   Efl_Boolean_Model *filter;
   Eina_Future *future;

   eina_value_setup(&v, EINA_VALUE_TYPE_INT);

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   ck_assert(!!base_model);

   for (i = 0; i < child_number; ++i)
     {
        child = efl_model_child_add(base_model);
        ck_assert(!!child);
        ck_assert(eina_value_set(&v, base_ints[i]));
        efl_model_property_set(child, "test_p_int", &v);
        efl_model_property_set(child, "original", eina_value_int_new(i));
     }

   model = efl_add_ref(EFL_BOOLEAN_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model),
                       efl_boolean_model_boolean_add(efl_added, "test_p_true", EINA_TRUE),
                       efl_boolean_model_boolean_add(efl_added, "test_p_false", EINA_FALSE),
                       efl_boolean_model_boolean_add(efl_added, "ready", EINA_FALSE));
   ck_assert(!!model);

   filter = efl_add_ref(EFL_FILTER_MODEL_CLASS, efl_main_loop_get(),
                        efl_ui_view_model_set(efl_added, model),
                        efl_filter_model_filter_set(efl_added, NULL, _filter_cb, NULL));
   ck_assert(!!filter);
   ck_assert_int_eq(efl_model_children_count_get(filter), 0);

   future = efl_model_children_slice_get(model, 0, efl_model_children_count_get(model));
   future = efl_future_then(model, future,
                            .success = _fetch_and_switch);
   future = efl_future_then(filter, future,
                            .success = _wait_count);
   future = efl_future_then(filter, future,
                            .success = _count_and_fetch);
   future = efl_future_then(filter, future,
                            .success = _check_index,
                            .free = _cleanup);

   ecore_main_loop_begin();

   efl_unref(filter);
   efl_del(filter);
}
EFL_END_TEST

static Eina_Value
_names_check(Eo *o EINA_UNUSED, void *data, const Eina_Value v)
{
   int *checked = data;
   unsigned int i, len;
   Efl_Model *child;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Eina_Value *name = efl_model_property_get(child, "name");
        Eina_Value *comment = efl_model_property_get(child, "comment");
        char *n = eina_value_to_string(name);
        char *c = eina_value_to_string(comment);

        ck_assert_str_eq(n, i ? "pear" : "apple");
        ck_assert_str_eq(c, "ripe");
        free(n);
        free(c);
        eina_value_free(name);
        eina_value_free(comment);
        (*checked)++;
     }
   return v;
}

/* A composite model is a view on its source: a property the source has is
   the source's, even when the wrapper is an Eo object with a property of the
   same name -- "name" and "comment" are Efl.Object's, and among the most
   common names a data model has. */
EFL_START_TEST(efl_test_composite_model_source_properties_first)
{
   Efl_Generic_Model *base_model, *child;
   Efl_Boolean_Model *model;
   Eina_Future *future;
   const char *names[] = { "apple", "pear" };
   int checked = 0;
   unsigned int i;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 2; i++)
     {
        Eina_Value *v;

        child = efl_model_child_add(base_model);
        v = eina_value_string_new(names[i]);
        efl_model_property_set(child, "name", v);
        eina_value_free(v);
        v = eina_value_string_new("ripe");
        efl_model_property_set(child, "comment", v);
        eina_value_free(v);
     }

   model = efl_add_ref(EFL_BOOLEAN_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model),
                       efl_boolean_model_boolean_add(efl_added, "selected", EINA_FALSE));

   future = efl_model_children_slice_get(model, 0, efl_model_children_count_get(model));
   future = efl_future_then(model, future,
                            .success = _names_check,
                            .success_type = EINA_VALUE_TYPE_ARRAY,
                            .data = &checked,
                            .free = _cleanup);
   ecore_main_loop_begin();

   ck_assert_int_eq(checked, 2);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

static void
_child_added_cb(void *data, const Efl_Event *event)
{
   Efl_Model_Children_Event *cevt = event->info;
   Efl_Model_Children_Event *seen = data;

   *seen = *cevt;
}

EFL_START_TEST(efl_test_generic_model_child_added)
{
   Efl_Generic_Model *model, *child;
   Efl_Model_Children_Event seen = { 0 };

   model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   efl_event_callback_add(model, EFL_MODEL_EVENT_CHILD_ADDED, _child_added_cb, &seen);

   efl_model_child_add(model);
   child = efl_model_child_add(model);

   ck_assert_int_eq(seen.index, 1);
   ck_assert_ptr_eq(seen.child, child);

   efl_unref(model);
}
EFL_END_TEST

/* A value type that counts: every setup must meet a flush, and it is larger than
 * eight bytes, so Eina gives it memory of its own as it does an array. */
static int _counted_live = 0;

static Eina_Bool
_counted_setup(const Eina_Value_Type *type EINA_UNUSED, void *mem)
{
   memset(mem, 0, 16);
   _counted_live++;
   return EINA_TRUE;
}

static Eina_Bool
_counted_flush(const Eina_Value_Type *type EINA_UNUSED, void *mem EINA_UNUSED)
{
   _counted_live--;
   return EINA_TRUE;
}

static Eina_Bool
_counted_copy(const Eina_Value_Type *type EINA_UNUSED, const void *src, void *dst)
{
   memcpy(dst, src, 16);
   return EINA_TRUE;
}

static int
_counted_compare(const Eina_Value_Type *type EINA_UNUSED, const void *a, const void *b)
{
   return memcmp(a, b, 16);
}

static Eina_Bool
_counted_pset(const Eina_Value_Type *type EINA_UNUSED, void *mem, const void *ptr)
{
   memcpy(mem, ptr, 16);
   return EINA_TRUE;
}

static Eina_Bool
_counted_pget(const Eina_Value_Type *type EINA_UNUSED, const void *mem, void *ptr)
{
   memcpy(ptr, mem, 16);
   return EINA_TRUE;
}

static const Eina_Value_Type _counted_type =
{
   EINA_VALUE_TYPE_VERSION, 16, "counted", _counted_setup, _counted_flush, _counted_copy,
   _counted_compare, NULL, NULL, NULL, _counted_pset, _counted_pget
};

static void
_set_counted(Efl_Generic_Model *model, const char *name, int tag)
{
   Eina_Value *v = eina_value_new(&_counted_type);
   int raw[4] = { tag, 0, 0, 0 };
   Eina_Future *f;

   ck_assert(v != NULL);
   ck_assert(eina_value_pset(v, raw));
   f = efl_model_property_set(model, name, v);
   eina_future_cancel(f);
   eina_value_free(v);
}

static void
_properties_changed(void *data, const Efl_Event *ev)
{
   Efl_Model_Property_Event *event = ev->info;
   Eina_Bool *invalidated_unset = data;

   *invalidated_unset = (event->invalidated_properties == NULL);
}

/* An event that carries a list of changed properties does not carry a stray
 * list of invalidated ones. The stray value is the stack's, so only an
 * uninitialized-read checker tells it reliably. */
EFL_START_TEST(efl_test_generic_model_properties_changed_event)
{
   Efl_Generic_Model *model;
   Eina_Bool invalidated_unset = EINA_FALSE;
   Eina_Value *v;

   model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   efl_event_callback_add(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED,
                          _properties_changed, &invalidated_unset);

   v = eina_value_int_new(42);
   eina_future_cancel(efl_model_property_set(model, "p", v));
   eina_value_free(v);
   ck_assert(invalidated_unset);

   efl_unref(model);
}
EFL_END_TEST

EFL_START_TEST(efl_test_generic_model_property_replaced)
{
   Efl_Generic_Model *model;
   Eina_Value *got;
   int i, raw[4];

   model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   _counted_live = 0;

   /* One value in the model: set it again and again, whatever it is. */
   for (i = 0; i < 4; i++)
     {
        _set_counted(model, "p", i);
        ck_assert_int_eq(_counted_live, 1);
        got = efl_model_property_get(model, "p");
        ck_assert(eina_value_type_get(got) == &_counted_type);
        ck_assert(eina_value_pget(got, raw));
        ck_assert_int_eq(raw[0], i);
        eina_value_free(got);
        ck_assert_int_eq(_counted_live, 1);
     }
   /* Another property: one more value. */
   _set_counted(model, "q", 7);
   ck_assert_int_eq(_counted_live, 2);

   /* What the model held goes with it (its parent, the loop, lets go first). */
   efl_del(model);
   efl_unref(model);
   ck_assert_int_eq(_counted_live, 0);
}
EFL_END_TEST

static Eina_Bool
_marked_named_get(Eo *child, const char *name)
{
   Eina_Value *r = efl_model_property_get(child, name);
   Eina_Bool b = EINA_FALSE;

   ck_assert(eina_value_bool_convert(r, &b));
   eina_value_free(r);
   return b;
}

static Eina_Bool
_marked_get(Eo *child)
{
   return _marked_named_get(child, "marked");
}

static void
_added_child_keep(void *data, const Efl_Event *ev)
{
   Efl_Model_Children_Event *cev = ev->info;
   Eo **kept = data;

   // Volatile children go once nobody holds them
   *kept = efl_ref(cev->child);
}

static Eina_Value
_insert_check(Eo *model, void *data EINA_UNUSED, const Eina_Value v)
{
   Efl_Model_Children_Event ev = { 0 };
   Eo *base = efl_ui_view_model_get(model);
   Eo *kids[5] = { NULL };
   Eo *sources[5] = { NULL };
   Eo *row, *added = NULL;
   unsigned int i, len;
   Eo *c;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, c)
     {
        ck_assert_int_lt(i, 5);
        kids[i] = c;
        sources[i] = efl_ui_view_model_get(c);
     }
   ck_assert_int_eq(len, 5);

   efl_model_property_set(kids[2], "marked", eina_value_bool_new(EINA_TRUE));
   ck_assert(_marked_get(kids[2]));

   // A new row in the middle, announced the way a model that can insert does it
   row = efl_add(EFL_GENERIC_MODEL_CLASS, base);
   ev.index = 2;
   ev.child = row;
   efl_event_callback_add(model, EFL_MODEL_EVENT_CHILD_ADDED, _added_child_keep, &added);
   efl_event_callback_call(base, EFL_MODEL_EVENT_CHILD_ADDED, &ev);
   efl_event_callback_del(model, EFL_MODEL_EVENT_CHILD_ADDED, _added_child_keep, &added);

   // The new row is the one at 2, unmarked, and is not the one that was there
   ck_assert_ptr_ne(added, NULL);
   ck_assert_ptr_ne(added, kids[2]);
   ck_assert_ptr_eq(efl_ui_view_model_get(added), row);
   ck_assert_int_eq(efl_composite_model_index_get(added), 2);
   ck_assert(!_marked_get(added));

   // The old 2 is 3 now, still its own row, and keeps its mark
   ck_assert_int_eq(efl_composite_model_index_get(kids[2]), 3);
   ck_assert_ptr_eq(efl_ui_view_model_get(kids[2]), sources[2]);
   ck_assert(_marked_get(kids[2]));

   // Those before are where they were, and those after are one further
   ck_assert_int_eq(efl_composite_model_index_get(kids[1]), 1);
   ck_assert(!_marked_get(kids[1]));
   ck_assert_int_eq(efl_composite_model_index_get(kids[3]), 4);
   ck_assert(!_marked_get(kids[3]));

   efl_unref(added);
   ecore_main_loop_quit();
   return v;
}

/* A child inserted in front of the others pushes them one index further, and
 * the marks go with the children rather than staying at their index.
 * Efl.Generic_Model only appends, so the insertion is announced the way a
 * model that can insert does it. */
EFL_START_TEST(efl_test_boolean_model_insert)
{
   Efl_Generic_Model *base_model;
   Efl_Boolean_Model *model;
   Eina_Future *future;
   int i;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 5; i++)
     {
        ck_assert(!!efl_model_child_add(base_model));
     }

   model = efl_add_ref(EFL_BOOLEAN_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model),
                       efl_boolean_model_boolean_add(efl_added, "marked", EINA_FALSE));
   ck_assert(!!model);

   future = efl_model_children_slice_get(model, 0, 5);
   efl_future_then(model, future, .success_type = EINA_VALUE_TYPE_ARRAY,
                   .success = _insert_check);

   ecore_main_loop_begin();

   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

static Eina_Value
_true_default_check(Eo *model EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value v)
{
   unsigned int i, len;
   Eo *c;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, c)
     {
        // Changing the first child allocates the buffer the others share
        if (!i) efl_model_property_set(c, "marked", eina_value_bool_new(EINA_FALSE));
        ck_assert(_marked_get(c) == !!i);
     }
   ck_assert_int_eq(len, 10);

   ecore_main_loop_quit();
   return v;
}

/* Marks across several storage ranges: 5000 children are three ranges of 2048
 * bits, and the marks sit on both sides of each border. Then a child is
 * inserted before the first border and another removed after it, and every
 * mark must still be on its row. Two booleans, one default false and one
 * default true, so that neither a missing nor a filled bit goes unnoticed. */
#define MANY 5000

static const unsigned int many_marks[] = { 0, 2047, 2048, 2049, 4500 };

static Eina_Bool
_many_is_mark(unsigned int id)
{
   unsigned int i;

   for (i = 0; i < EINA_C_ARRAY_LENGTH(many_marks); i++)
     {
        if (many_marks[i] == id) return EINA_TRUE;
     }
   return EINA_FALSE;
}

static void
_many_set(Eo *child, const char *name, Eina_Bool value)
{
   eina_future_cancel(efl_model_property_set(child, name, eina_value_bool_new(value)));
}

static Eina_Value
_many_check(Eo *model, void *data EINA_UNUSED, const Eina_Value v)
{
   Efl_Model_Children_Event ev = { 0 };
   Eo *base = efl_ui_view_model_get(model);
   Eo **kids = calloc(MANY, sizeof (Eo *));
   Eo *row;
   Eo *added = NULL;
   unsigned int i, len, id;
   Eo *c;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, c)
     {
        kids[i] = c;
     }
   ck_assert_int_eq(len, MANY);

   // "a" starts false and the marks make it true, "b" starts true and they make it false
   for (i = 0; i < EINA_C_ARRAY_LENGTH(many_marks); i++)
     {
        _many_set(kids[many_marks[i]], "a", EINA_TRUE);
        _many_set(kids[many_marks[i]], "b", EINA_FALSE);
     }

   // In front of the first border
   row = efl_add(EFL_GENERIC_MODEL_CLASS, base);
   ev.index = 100;
   ev.child = row;
   efl_event_callback_add(model, EFL_MODEL_EVENT_CHILD_ADDED, _added_child_keep, &added);
   efl_event_callback_call(base, EFL_MODEL_EVENT_CHILD_ADDED, &ev);
   efl_event_callback_del(model, EFL_MODEL_EVENT_CHILD_ADDED, _added_child_keep, &added);
   ck_assert_ptr_ne(added, NULL);
   ck_assert_int_eq(efl_composite_model_index_get(added), 100);
   ck_assert(!_marked_named_get(added, "a"));
   ck_assert(_marked_named_get(added, "b"));

   // After it, on the child that is now at 2050 (it was 2049, which carried a mark)
   ev.index = 2050;
   ev.child = NULL;
   efl_event_callback_call(base, EFL_MODEL_EVENT_CHILD_REMOVED, &ev);

   for (id = 0; id < MANY; id++)
     {
        unsigned int index = (id < 100) ? id : (id + 1);

        if (id == 2049) continue; // gone, with its mark
        if (id > 2049) index--;
        ck_assert_int_eq(efl_composite_model_index_get(kids[id]), index);
        ck_assert(_marked_named_get(kids[id], "a") == _many_is_mark(id));
        ck_assert(_marked_named_get(kids[id], "b") == !_many_is_mark(id));
     }
   ck_assert(!_marked_named_get(added, "a"));
   ck_assert(_marked_named_get(added, "b"));

   efl_unref(added);
   free(kids);
   ecore_main_loop_quit();
   return v;
}

/* A boolean that is true by default is true for every child sharing a buffer
 * with one that was changed, not only for the first of each byte. */
EFL_START_TEST(efl_test_boolean_model_true_default)
{
   Efl_Generic_Model *base_model;
   Efl_Boolean_Model *model;
   int i;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 10; i++)
     {
        ck_assert(!!efl_model_child_add(base_model));
     }

   model = efl_add_ref(EFL_BOOLEAN_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model),
                       efl_boolean_model_boolean_add(efl_added, "marked", EINA_TRUE));
   ck_assert(!!model);

   efl_future_then(model, efl_model_children_slice_get(model, 0, 10),
                   .success_type = EINA_VALUE_TYPE_ARRAY,
                   .success = _true_default_check);
   ecore_main_loop_begin();

   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

EFL_START_TEST(efl_test_boolean_model_many)
{
   Efl_Generic_Model *base_model;
   Efl_Boolean_Model *model;
   Eina_Future *future;
   int i;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < MANY; i++)
     {
        ck_assert(!!efl_model_child_add(base_model));
     }

   model = efl_add_ref(EFL_BOOLEAN_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model),
                       efl_boolean_model_boolean_add(efl_added, "a", EINA_FALSE),
                       efl_boolean_model_boolean_add(efl_added, "b", EINA_TRUE));
   ck_assert(!!model);

   future = efl_model_children_slice_get(model, 0, MANY);
   efl_future_then(model, future, .success_type = EINA_VALUE_TYPE_ARRAY,
                   .success = _many_check);

   ecore_main_loop_begin();

   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// Rows taken out from the front of a list, the marks sit close to it. Every mark
// must stay on its row, and the rows that were never marked must not get one.
// Two booleans, one default false and one default true. The bits past the last
// stored range are the default and are not moved, so the marks sit in the first
// range, on both sides of the border of the second, and past the last stored one
// nothing is marked.
#define FRONT_TOTAL 5000
#define FRONT_GONE 300

static const unsigned int front_marks[] = { 0, 299, 300, 1500, 2047, 2048, 2049 };
static Eo **front_rows;
static unsigned int front_total;

static Eina_Bool
_front_is_mark(unsigned int id)
{
   unsigned int i;

   for (i = 0; i < EINA_C_ARRAY_LENGTH(front_marks); i++)
     {
        if (front_marks[i] == id) return EINA_TRUE;
     }
   return EINA_FALSE;
}

static Eina_Value
_front_check(Eo *model, void *data EINA_UNUSED, const Eina_Value v)
{
   Eo *base = efl_ui_view_model_get(model);
   Eo **kids = calloc(front_total, sizeof (Eo *));
   unsigned int i, len, id;
   Eo *c;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, c)
     {
        kids[i] = c;
     }
   ck_assert_int_eq(len, front_total);

   for (i = 0; i < EINA_C_ARRAY_LENGTH(front_marks); i++)
     {
        _many_set(kids[front_marks[i]], "a", EINA_TRUE);
        _many_set(kids[front_marks[i]], "b", EINA_FALSE);
     }

   for (id = 0; id < FRONT_GONE; id++)
     {
        efl_model_child_del(base, front_rows[id]);
     }

   for (id = FRONT_GONE; id < front_total; id++)
     {
        ck_assert_int_eq(efl_composite_model_index_get(kids[id]), id - FRONT_GONE);
        ck_assert(_marked_named_get(kids[id], "a") == _front_is_mark(id));
        ck_assert(_marked_named_get(kids[id], "b") == !_front_is_mark(id));
     }
   ck_assert(_marked_named_get(kids[front_total - 1], "b"));
   ck_assert(!_marked_named_get(kids[front_total - 1], "a"));

   free(kids);
   ecore_main_loop_quit();
   return v;
}

static void
_front_run(unsigned int total)
{
   Efl_Generic_Model *base_model;
   Efl_Boolean_Model *model;
   Eina_Future *future;
   unsigned int i;

   front_total = total;
   front_rows = calloc(total, sizeof (Eo *));
   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < total; i++)
     {
        front_rows[i] = efl_model_child_add(base_model);
        ck_assert(!!front_rows[i]);
     }

   model = efl_add_ref(EFL_BOOLEAN_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model),
                       efl_boolean_model_boolean_add(efl_added, "a", EINA_FALSE),
                       efl_boolean_model_boolean_add(efl_added, "b", EINA_TRUE));
   ck_assert(!!model);

   future = efl_model_children_slice_get(model, 0, total);
   efl_future_then(model, future, .success_type = EINA_VALUE_TYPE_ARRAY,
                   .success = _front_check);

   ecore_main_loop_begin();

   efl_unref(model);
   efl_unref(base_model);
   free(front_rows);
}

EFL_START_TEST(efl_test_boolean_model_remove_front)
{
   _front_run(FRONT_TOTAL);
}
EFL_END_TEST

// A slice that was asked for before rows came or went in front of it. The source
// answers with the rows it had when asked, so their composites must take the index
// those rows have now, not the one they had when the slice was asked for.
#define PENDING_MAX 8

typedef struct _Pending_Slice Pending_Slice;
struct _Pending_Slice
{
   Eo *kids[PENDING_MAX];
   unsigned int len;
   Eina_Bool done;
};

static Eina_Value
_pending_slice_then(void *data, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   Pending_Slice *ps = data;
   unsigned int i, len;
   Eo *c;

   ps->done = EINA_TRUE;
   if (eina_value_type_get(&v) != EINA_VALUE_TYPE_ARRAY) return v;
   EINA_VALUE_ARRAY_FOREACH(&v, len, i, c)
     {
        if (i < PENDING_MAX) ps->kids[i] = efl_ref(c);
     }
   ps->len = len;
   return v;
}

static void
_pending_slice_get(Eo *model, unsigned int start, unsigned int count, Pending_Slice *ps)
{
   memset(ps, 0, sizeof (*ps));
   eina_future_then(efl_model_children_slice_get(model, start, count),
                    _pending_slice_then, ps, NULL);
   // Not answered yet: the source answers on the loop
   ck_assert(!ps->done);
}

static void
_pending_slice_wait(Pending_Slice *ps)
{
   unsigned int i;

   for (i = 0; (i < 20) && !ps->done; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert(ps->done);
}

static void
_pending_slice_unref(Pending_Slice *ps)
{
   unsigned int i;

   for (i = 0; i < PENDING_MAX; i++)
     {
        efl_unref(ps->kids[i]);
     }
   memset(ps, 0, sizeof (*ps));
}

static Eo *
_pending_setup(Eo **base, Eo **rows, unsigned int n)
{
   Pending_Slice ps;
   unsigned int i;

   *base = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < n; i++)
     {
        rows[i] = efl_model_child_add(*base);
        ck_assert(!!rows[i]);
     }
   // (the rows themselves, as the generic model gives them)
   _pending_slice_get(*base, 0, n, &ps);
   _pending_slice_wait(&ps);
   _pending_slice_unref(&ps);

   return efl_add_ref(EFL_BOOLEAN_MODEL_CLASS, efl_main_loop_get(),
                      efl_ui_view_model_set(efl_added, *base),
                      efl_boolean_model_boolean_add(efl_added, "marked", EINA_FALSE));
}

static unsigned int
_marked_indexes(Eo *model, unsigned int *out, unsigned int max)
{
   Eina_Iterator *it = efl_boolean_model_boolean_iterator_get(model, "marked", EINA_TRUE);
   unsigned int *index, n = 0;

   EINA_ITERATOR_FOREACH(it, index)
     {
        if (n < max) out[n] = *index;
        n++;
     }
   eina_iterator_free(it);
   return n;
}

EFL_START_TEST(efl_test_composite_model_pending_slice_insert)
{
   Efl_Model_Children_Event ev = { 0 };
   Pending_Slice held, ps;
   Eo *base, *model, *rows[6], *row;
   unsigned int marked[4];

   model = _pending_setup(&base, rows, 6);

   // Row 4 already has its composite
   _pending_slice_get(model, 4, 1, &held);
   _pending_slice_wait(&held);
   ck_assert_int_eq(efl_composite_model_index_get(held.kids[0]), 4);

   // Rows 3 and 4 are asked for, then a row comes in front of them
   _pending_slice_get(model, 3, 2, &ps);
   row = efl_add(EFL_GENERIC_MODEL_CLASS, base);
   ev.index = 0;
   ev.child = row;
   efl_event_callback_call(base, EFL_MODEL_EVENT_CHILD_ADDED, &ev);
   ck_assert_int_eq(efl_composite_model_index_get(held.kids[0]), 5);
   _pending_slice_wait(&ps);

   ck_assert_int_eq(ps.len, 2);
   ck_assert_ptr_eq(efl_ui_view_model_get(ps.kids[0]), rows[3]);
   ck_assert_int_eq(efl_composite_model_index_get(ps.kids[0]), 4);
   ck_assert_ptr_eq(ps.kids[1], held.kids[0]);
   ck_assert_int_eq(efl_composite_model_index_get(ps.kids[1]), 5);

   // A mark on the row lands on its own index, and on nobody else
   eina_future_cancel(efl_model_property_set(ps.kids[0], "marked", eina_value_bool_new(EINA_TRUE)));
   ck_assert(_marked_get(ps.kids[0]));
   ck_assert(!_marked_get(held.kids[0]));
   ck_assert_int_eq(_marked_indexes(model, marked, 4), 1);
   ck_assert_int_eq(marked[0], 4);

   _pending_slice_unref(&ps);
   _pending_slice_unref(&held);
   efl_unref(model);
   efl_unref(base);
}
EFL_END_TEST

EFL_START_TEST(efl_test_composite_model_pending_slice_remove)
{
   Pending_Slice held, ps;
   Eo *base, *model, *rows[6];

   model = _pending_setup(&base, rows, 6);

   // Row 4 has its composite; after the removal it sits at 3, where row 3 was asked for
   _pending_slice_get(model, 4, 1, &held);
   _pending_slice_wait(&held);

   _pending_slice_get(model, 3, 1, &ps);
   efl_model_child_del(base, rows[0]);
   ck_assert_int_eq(efl_composite_model_index_get(held.kids[0]), 3);
   _pending_slice_wait(&ps);

   // Row 3 is its own composite at 2, not row 4's
   ck_assert_int_eq(ps.len, 1);
   ck_assert_ptr_ne(ps.kids[0], held.kids[0]);
   ck_assert_ptr_eq(efl_ui_view_model_get(ps.kids[0]), rows[3]);
   ck_assert_int_eq(efl_composite_model_index_get(ps.kids[0]), 2);
   ck_assert_ptr_eq(efl_ui_view_model_get(held.kids[0]), rows[4]);
   ck_assert_int_eq(efl_composite_model_index_get(held.kids[0]), 3);

   _pending_slice_unref(&ps);
   _pending_slice_unref(&held);
   efl_unref(model);
   efl_unref(base);
}
EFL_END_TEST

EFL_START_TEST(efl_test_composite_model_pending_slice_removed_row)
{
   Pending_Slice ps;
   Eo *base, *model, *rows[4];
   unsigned int marked[4];

   model = _pending_setup(&base, rows, 4);

   // Row 0 is asked for and goes before the answer
   _pending_slice_get(model, 0, 2, &ps);
   efl_model_child_del(base, rows[0]);
   _pending_slice_wait(&ps);

   // The answer keeps its length: the removed row has no index, row 1 is at 0
   ck_assert_int_eq(ps.len, 2);
   ck_assert_ptr_eq(efl_ui_view_model_get(ps.kids[0]), rows[0]);
   ck_assert_int_eq(efl_composite_model_index_get(ps.kids[0]), 0xFFFFFFFF);
   ck_assert_ptr_eq(efl_ui_view_model_get(ps.kids[1]), rows[1]);
   ck_assert_int_eq(efl_composite_model_index_get(ps.kids[1]), 0);

   // Marking a row that is gone marks nothing
   eina_future_cancel(efl_model_property_set(ps.kids[0], "marked", eina_value_bool_new(EINA_TRUE)));
   ck_assert_int_eq(_marked_indexes(model, marked, 4), 0);
   ck_assert(!_marked_get(ps.kids[1]));

   _pending_slice_unref(&ps);
   efl_unref(model);
   efl_unref(base);
}
EFL_END_TEST

// A slice whose count wraps start + count around asks for nothing that exists.
static Eina_Value
_wrap_error(void *data, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   Eina_Error *err = data;

   if (eina_value_type_get(&v) == EINA_VALUE_TYPE_ERROR)
     eina_value_error_get(&v, err);
   else
     *err = 0;
   return v;
}

EFL_START_TEST(efl_test_composite_model_slice_count_wraps)
{
   Eina_Error err = -1;
   Eo *base, *model, *rows[6];
   unsigned int i;

   model = _pending_setup(&base, rows, 6);
   eina_future_then(efl_model_children_slice_get(model, 10, UINT_MAX - 5),
                    _wrap_error, &err, NULL);
   for (i = 0; (i < 8) && (err == -1); i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(err, EFL_MODEL_ERROR_INCORRECT_VALUE);

   efl_unref(model);
   efl_unref(base);
}
EFL_END_TEST

/* The data given to filter_set is the model's: it is freed when the filter is
 * replaced and when the model goes, through the callback given with it. */
static int filter_data_freed;

static void
_filter_data_free(void *data EINA_UNUSED)
{
   filter_data_freed++;
}

static Eina_Future *
_filter_never_asked(void *data EINA_UNUSED, Efl_Filter_Model *parent EINA_UNUSED,
                    Efl_Model *child EINA_UNUSED)
{
   return NULL;
}

EFL_START_TEST(efl_test_filter_model_frees_filter_data)
{
   Efl_Generic_Model *base;
   Efl_Filter_Model *filter;

   filter_data_freed = 0;

   base = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   ck_assert(!!base);

   filter = efl_add_ref(EFL_FILTER_MODEL_CLASS, efl_main_loop_get(),
                        efl_ui_view_model_set(efl_added, base),
                        efl_filter_model_filter_set(efl_added, &filter_data_freed,
                                                    _filter_never_asked, _filter_data_free));
   ck_assert(!!filter);
   ck_assert_int_eq(filter_data_freed, 0);

   efl_filter_model_filter_set(filter, &filter_data_freed,
                               _filter_never_asked, _filter_data_free);
   ck_assert_int_eq(filter_data_freed, 1);

   efl_unref(filter);
   efl_del(filter);
   ck_assert_int_eq(filter_data_freed, 2);

   efl_unref(base);
   efl_del(base);
}
EFL_END_TEST

// A row that leaves is still the row it was while the removal is told, and has
// no place in the model after it, for every model that wraps another one.
typedef struct _Gone_Watch Gone_Watch;
struct _Gone_Watch
{
   Eo *row;
   unsigned int during;
   unsigned int told;
};

static void
_gone_watch_removed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Gone_Watch *gw = data;

   gw->during = efl_composite_model_index_get(gw->row);
   gw->told++;
}

static Eina_Future *
_gone_filter_cb(void *data EINA_UNUSED, Efl_Filter_Model *parent, Efl_Model *child EINA_UNUSED)
{
   return efl_loop_future_resolved(parent, eina_value_bool_init(EINA_TRUE));
}

static void
_gone_remove_check(Eo *model, Eo *base, Eo **rows, Pending_Slice *ps,
                   unsigned int at, unsigned int shown)
{
   Gone_Watch gw = { 0 };

   gw.row = ps->kids[shown];
   ck_assert_ptr_ne(gw.row, NULL);
   ck_assert_uint_eq(efl_composite_model_index_get(gw.row), shown);
   efl_event_callback_add(model, EFL_MODEL_EVENT_CHILD_REMOVED, _gone_watch_removed, &gw);
   efl_model_child_del(base, rows[at]);
   efl_event_callback_del(model, EFL_MODEL_EVENT_CHILD_REMOVED, _gone_watch_removed, &gw);

   ck_assert_uint_eq(gw.told, 1);
   ck_assert_uint_eq(gw.during, shown);
   ck_assert_uint_eq(efl_composite_model_index_get(gw.row), 0xFFFFFFFF);
}

EFL_START_TEST(efl_test_filter_model_removed_row_gone)
{
   Pending_Slice ps;
   Eo *base, *filter, *rows[6];
   unsigned int i;

   base = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 6; i++)
     {
        rows[i] = efl_model_child_add(base);
        ck_assert(!!rows[i]);
     }
   filter = efl_add_ref(EFL_FILTER_MODEL_CLASS, efl_main_loop_get(),
                        efl_ui_view_model_set(efl_added, base),
                        efl_filter_model_filter_set(efl_added, NULL, _gone_filter_cb, NULL));
   ck_assert(!!filter);
   // (the filter counts when asked, and the rows come as the loop runs)
   efl_model_children_count_get(filter);
   for (i = 0; (i < 50) && efl_model_children_count_get(filter) < 6; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(efl_model_children_count_get(filter), 6);

   _pending_slice_get(filter, 0, 6, &ps);
   _pending_slice_wait(&ps);
   ck_assert_uint_eq(ps.len, 6);

   _gone_remove_check(filter, base, rows, &ps, 2, 2);
   // The one after it is one closer
   ck_assert_uint_eq(efl_composite_model_index_get(ps.kids[3]), 2);

   _pending_slice_unref(&ps);
   efl_unref(filter);
   efl_unref(base);
}
EFL_END_TEST

void
efl_test_case_boolean_model(TCase *tc)
{
   tcase_add_test(tc, efl_test_boolean_model);
   tcase_add_test(tc, efl_test_boolean_model_insert);
   tcase_add_test(tc, efl_test_boolean_model_true_default);
   tcase_add_test(tc, efl_test_boolean_model_many);
   tcase_add_test(tc, efl_test_boolean_model_remove_front);
   tcase_add_test(tc, efl_test_filter_model);
   tcase_add_test(tc, efl_test_generic_model_child_added);
   tcase_add_test(tc, efl_test_composite_model_source_properties_first);
   tcase_add_test(tc, efl_test_generic_model_properties_changed_event);
   tcase_add_test(tc, efl_test_generic_model_property_replaced);
   tcase_add_test(tc, efl_test_composite_model_pending_slice_insert);
   tcase_add_test(tc, efl_test_composite_model_pending_slice_remove);
   tcase_add_test(tc, efl_test_composite_model_pending_slice_removed_row);
   tcase_add_test(tc, efl_test_composite_model_slice_count_wraps);
   tcase_add_test(tc, efl_test_filter_model_removed_row_gone);
   tcase_add_test(tc, efl_test_filter_model_frees_filter_data);
}
