#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "efl_ui_suite.h"
#include "efl_ui_homogeneous_model.eo.h"
#include "efl_ui_exact_model.eo.h"
#include "efl_ui_average_model.eo.h"

static const int child_number = 3;
static const int base_ints[] = { 41, 42, 43 };
static int expected_average[] = { 0, 0, 0 };

static Efl_Model *
_generate_base_model(void)
{
   Efl_Generic_Model *base_model, *child;
   Eina_Value v;
   int i;

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
   eina_value_flush(&v);

   return base_model;
}

static Eina_Error
_property_error_expected(Efl_Model *model, const char *property)
{
   Eina_Value *v;
   Eina_Error err = 0;

   v = efl_model_property_get(model, property);
   ck_assert_ptr_eq(eina_value_type_get(v), EINA_VALUE_TYPE_ERROR);
   ck_assert(eina_value_error_get(v, &err));
   eina_value_free(v);

   return err;
}

static unsigned int
_property_uint_expected(Efl_Model *model, const char *property)
{
   Eina_Value *v;
   unsigned int r = 0;

   v = efl_model_property_get(model, property);
   ck_assert_ptr_eq(eina_value_type_get(v), EINA_VALUE_TYPE_UINT);
   ck_assert(eina_value_uint_get(v, &r));
   eina_value_free(v);

   return r;
}

static Eina_Value
_child_should_succeed(Efl_Model *model EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value v)
{
   ck_assert_ptr_eq(eina_value_type_get(&v), EINA_VALUE_TYPE_ARRAY);
   return v;
}

static Eina_Value
_child_should_fail(Efl_Model *model EINA_UNUSED, void *data, const Eina_Value v)
{
   unsigned int i, len;
   Eina_Value c = EINA_VALUE_EMPTY;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, c)
     {
        if (eina_value_type_get(&c) != EINA_VALUE_TYPE_ERROR)
          {
             fprintf(stderr, "Request on child %i should have failed but got '%s'\n",
                     (int)(uintptr_t) data, eina_value_to_string(&c));
             abort();
          }
     }
   return eina_value_int_init(0);
}

static Eina_Value
_total_succeed(Efl_Model *model EINA_UNUSED, void *data EINA_UNUSED, const Eina_Value v)
{
   efl_loop_quit(efl_loop_get(model), eina_value_string_init("BOOM"));
   return v;
}

static Eina_Value
_total_failed(Efl_Model *model EINA_UNUSED, void *data EINA_UNUSED, Eina_Error err)
{
   fprintf(stderr, "Shouldn't have failed: '%s'\n", eina_error_msg_get(err));
   efl_loop_quit(efl_loop_get(model), eina_value_int_init(42));
   return eina_value_error_init(err);
}

static Eina_Value
_children_homogeneous_slice_get_then(Efl_Model *model, void *data EINA_UNUSED, const Eina_Value v)
{
   unsigned int i, len;
   Efl_Model *child = NULL;
   Eina_Future *all[4] = { NULL, NULL, NULL, EINA_FUTURE_SENTINEL };
   Eina_Future *f;

   fail_if(eina_value_type_get(&v) != EINA_VALUE_TYPE_ARRAY);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        ck_assert_int_eq(_property_error_expected(child, "self.width"), EAGAIN);
        ck_assert_int_eq(_property_error_expected(child, "self.height"), EAGAIN);
     }

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Eina_Value *v;
        unsigned int w, h;

        v = efl_model_property_get(child, "test_p_int");
        eina_value_uint_convert(v, &w);
        eina_value_uint_convert(v, &h);
        eina_value_free(v);

        w *= 2;
        h *= 3;

        all[i] = eina_future_all(efl_model_property_set(child, "self.width", eina_value_uint_new(w)),
                                 efl_model_property_set(child, "self.height", eina_value_uint_new(h)));

        if (i == 0)
          all[i] = efl_future_then(model, all[i], .success = _child_should_succeed);
        else
          all[i] = efl_future_then(model, all[i],
                                   .success = _child_should_fail,
                                   .success_type = EINA_VALUE_TYPE_ARRAY,
                                   .data = (void*)(uintptr_t) i);
     }

   f = eina_future_all_array(all);
   f = efl_future_then(model, f, .success = _total_succeed, .error = _total_failed);
   return eina_future_as_value(f);
}

EFL_START_TEST(efl_ui_homogeneous_model_test)
{
   Efl_Generic_Model *base_model, *model;
   Eina_Value *ret__;
   Eina_Future *future;
   int real__;

   base_model = _generate_base_model();

   model = efl_add_ref(EFL_UI_HOMOGENEOUS_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);

   future = efl_model_children_slice_get(model, 0, efl_model_children_count_get(model));
   efl_future_then(model, future, .success = _children_homogeneous_slice_get_then);

   ck_assert_int_eq(_property_error_expected(model, "total.width"), EAGAIN);
   ck_assert_int_eq(_property_error_expected(model, "total.height"), EAGAIN);
   ck_assert_int_eq(_property_error_expected(model, "item.width"), EAGAIN);
   ck_assert_int_eq(_property_error_expected(model, "item.height"), EAGAIN);

   ret__ = efl_loop_begin(efl_app_main_get());
   real__ = efl_loop_exit_code_process(ret__);
   // (what the loop quit with is the test's to flush, once)
   eina_value_flush(ret__);
   *ret__ = EINA_VALUE_EMPTY;
   fail_if(real__ != 0);

   ck_assert_int_eq(_property_uint_expected(model, "total.width"), base_ints[0] * 2);
   ck_assert_int_eq(_property_uint_expected(model, "total.height"), base_ints[0] * 3 * 3);
   ck_assert_int_eq(_property_uint_expected(model, "item.width"), base_ints[0] * 2);
   ck_assert_int_eq(_property_uint_expected(model, "item.height"), base_ints[0] * 3);

   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

static Eina_Value
_children_exact_slice_get_then(Efl_Model *model, void *data EINA_UNUSED, const Eina_Value v)
{
   unsigned int i, len;
   Efl_Model *child = NULL;
   Eina_Future *all[4] = { NULL, NULL, NULL, EINA_FUTURE_SENTINEL };
   Eina_Future *f;

   fail_if(eina_value_type_get(&v) != EINA_VALUE_TYPE_ARRAY);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        // (not measured: not ready, which is not a size of 0)
        ck_assert_int_eq(_property_error_expected(child, "self.width"), EAGAIN);
        ck_assert_int_eq(_property_error_expected(child, "self.height"), EAGAIN);
     }

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Eina_Value *v;
        unsigned int w, h;

        v = efl_model_property_get(child, "test_p_int");
        eina_value_uint_convert(v, &w);
        eina_value_uint_convert(v, &h);
        eina_value_free(v);

        w *= 2;
        h *= 3;

        all[i] = eina_future_all(efl_model_property_set(child, "self.width", eina_value_uint_new(w)),
                                 efl_model_property_set(child, "self.height", eina_value_uint_new(h)));
        all[i] = efl_future_then(model, all[i], .success = _child_should_succeed);
     }

   f = eina_future_all_array(all);
   f = efl_future_then(model, f, .success = _total_succeed, .error = _total_failed);
   return eina_future_as_value(f);
}

EFL_START_TEST(efl_ui_exact_model_test)
{
   Efl_Generic_Model *base_model, *model;
   Eina_Future *future;
   Eina_Value *ret__;
   int real__;

   base_model = _generate_base_model();

   model = efl_add_ref(EFL_UI_EXACT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);

   future = efl_model_children_slice_get(model, 0, efl_model_children_count_get(model));
   efl_future_then(model, future, .success = _children_exact_slice_get_then);

   ck_assert_int_eq(_property_uint_expected(model, "total.width"), 0);
   ck_assert_int_eq(_property_uint_expected(model, "total.height"), 0);
   ck_assert_int_eq(_property_error_expected(model, "item.width"), EAGAIN);
   ck_assert_int_eq(_property_error_expected(model, "item.height"), EAGAIN);

   ret__ = efl_loop_begin(efl_app_main_get());
   real__ = efl_loop_exit_code_process(ret__);
   // (what the loop quit with is the test's to flush, once)
   eina_value_flush(ret__);
   *ret__ = EINA_VALUE_EMPTY;
   fail_if(real__ != 0);

   ck_assert_int_eq(_property_uint_expected(model, "total.width"), base_ints[2] * 2);
   ck_assert_int_eq(_property_uint_expected(model, "total.height"), base_ints[0] * 3 + base_ints[1] * 3 + base_ints[2] * 3);

   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

static Eina_Value
_child_updated_average(Efl_Model *model, void *data, const Eina_Value v)
{
   Eina_Value *r;
   int *ex = data;
   int got = 0;

   r = efl_model_property_get(model, "total.height");
   ck_assert(eina_value_int_convert(r, &got));
   ck_assert_int_eq(got, *ex);
   eina_value_free(r);

   return v;
}

static Eina_Value
_children_average_slice_get_then(Efl_Model *model, void *data EINA_UNUSED, const Eina_Value v)
{
   unsigned int i, len;
   Efl_Model *child = NULL;
   Eina_Future *all[4] = { NULL, NULL, NULL, EINA_FUTURE_SENTINEL };
   Eina_Future *f;

   fail_if(eina_value_type_get(&v) != EINA_VALUE_TYPE_ARRAY);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        ck_assert_int_eq(_property_error_expected(child, "self.width"), EAGAIN);
        ck_assert_int_eq(_property_error_expected(child, "self.height"), EAGAIN);
     }

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Eina_Value *v;
        unsigned int w, h;

        v = efl_model_property_get(child, "test_p_int");
        eina_value_uint_convert(v, &w);
        eina_value_uint_convert(v, &h);
        eina_value_free(v);

        w *= 2;
        h *= 3;

        all[i] = eina_future_all(efl_model_property_set(child, "self.width", eina_value_uint_new(w)),
                                 efl_model_property_set(child, "self.height", eina_value_uint_new(h)));
        all[i] = efl_future_then(model, all[i], .success = _child_should_succeed);

        // The model average is not updated until the property have succeeded, so we have
        // to wait before we can fetch the property.
        all[i] = efl_future_then(model, all[i], .success = _child_updated_average, .data = &expected_average[i]);
     }

   f = eina_future_all_array(all);
   f = efl_future_then(model, f, .success = _total_succeed, .error = _total_failed);
   return eina_future_as_value(f);
}

EFL_START_TEST(efl_ui_average_model_test)
{
   Efl_Generic_Model *base_model, *model;
   Eina_Future *future;
   Eina_Value *ret__;
   int real__;

   base_model = _generate_base_model();

   model = efl_add_ref(EFL_UI_AVERAGE_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);

   future = efl_model_children_slice_get(model, 0, efl_model_children_count_get(model));
   efl_future_then(model, future, .success = _children_average_slice_get_then);

   ck_assert_int_eq(_property_uint_expected(model, "total.width"), 0);
   ck_assert_int_eq(_property_uint_expected(model, "total.height"), 0);
   ck_assert_int_eq(_property_error_expected(model, "item.width"), EAGAIN);
   ck_assert_int_eq(_property_error_expected(model, "item.height"), EAGAIN);

   ret__ = efl_loop_begin(efl_app_main_get());
   real__ = efl_loop_exit_code_process(ret__);
   // (what the loop quit with is the test's to flush, once)
   eina_value_flush(ret__);
   *ret__ = EINA_VALUE_EMPTY;
   fail_if(real__ != 0);

   ck_assert_int_eq(_property_uint_expected(model, "total.width"), base_ints[2] * 2);
   ck_assert_int_eq(_property_uint_expected(model, "total.height"), base_ints[0] * 3 + base_ints[1] * 3 + base_ints[2] * 3);

   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// A row that left while the slice holding it was pending has no index (0xFFFFFFFF):
// it has no size, takes none, and leaves the total alone.
static Eina_Value
_gone_kids_then(void *data, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   Eo **kids = data;
   unsigned int i, len;
   Eo *c;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, c)
     {
        if (i < 2) kids[i] = efl_ref(c);
     }
   return v;
}

static Eina_Value
_gone_set_then(void *data, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   Eina_Error *err = data;

   *err = 0;
   if (eina_value_type_get(&v) == EINA_VALUE_TYPE_ERROR) eina_value_error_get(&v, err);
   return v;
}

EFL_START_TEST(efl_ui_exact_model_gone_row)
{
   Eina_Value told = eina_value_uint_init(10);
   Efl_Model *base_model, *model, *row0;
   Eo *kids[2] = { NULL, NULL };
   Eina_Error err = -1;
   unsigned int i;

   base_model = _generate_base_model();
   model = efl_add_ref(EFL_UI_EXACT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);

   eina_future_then(efl_model_children_slice_get(base_model, 0, 1), _gone_kids_then, kids, NULL);
   for (i = 0; (i < 8) && !kids[0]; i++)
     {
        ecore_main_loop_iterate();
     }
   row0 = kids[0];
   kids[0] = NULL;
   ck_assert(!!row0);

   eina_future_then(efl_model_children_slice_get(model, 0, 2), _gone_kids_then, kids, NULL);
   efl_model_child_del(base_model, row0);
   for (i = 0; (i < 8) && !kids[1]; i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert(!!kids[0]);
   ck_assert_int_eq(efl_composite_model_index_get(kids[0]), 0xFFFFFFFF);

   ck_assert_int_eq(_property_error_expected(kids[0], "self.height"), EAGAIN);
   eina_future_then(efl_model_property_set(kids[0], "self.height", &told),
                    _gone_set_then, &err, NULL);
   for (i = 0; (i < 8) && (err == -1); i++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(err, EFL_MODEL_ERROR_INCORRECT_VALUE);
   ck_assert_int_eq(_property_uint_expected(model, "total.height"), 0);

   eina_value_flush(&told);
   efl_unref(row0);
   efl_unref(kids[0]);
   efl_unref(kids[1]);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// Rows past the first block of 1024 are kept in other slots, and a slot that is
// reused for another block must be written back under its own block.
#define MANY_ROWS 3500

typedef struct _Many_Rows Many_Rows;

struct _Many_Rows
{
   Eo *rows[MANY_ROWS];
   unsigned int count;
};

static Eina_Value
_many_rows_then(void *data, const Eina_Value v, const Eina_Future *dead EINA_UNUSED)
{
   Many_Rows *many = data;
   unsigned int i, len;
   Eo *c;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, c)
     {
        many->rows[i] = efl_ref(c);
        many->count++;
     }
   return v;
}

static unsigned int
_many_height(unsigned int i)
{
   return 1 + ((i * 7) % 97) + ((i / 1024) * 100);
}

EFL_START_TEST(efl_ui_exact_model_many_rows)
{
   Efl_Generic_Model *base_model;
   Efl_Model *model;
   Many_Rows *many = calloc(1, sizeof (Many_Rows));
   unsigned int i, step, sum = 0, loops;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < MANY_ROWS; i++)
     {
        efl_model_child_add(base_model);
     }
   model = efl_add_ref(EFL_UI_EXACT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);

   eina_future_then(efl_model_children_slice_get(model, 0, MANY_ROWS), _many_rows_then, many, NULL);
   for (loops = 0; (loops < 50) && (many->count < MANY_ROWS); loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(many->count, MANY_ROWS);

   // scattered: every block is left and come back to many times
   for (step = 0, i = 0; step < MANY_ROWS; step++, i = (i + 1237) % MANY_ROWS)
     {
        efl_future_then(many->rows[i], efl_model_property_set(many->rows[i], "self.height",
                                                              eina_value_uint_new(_many_height(i))));
        sum += _many_height(i);
     }
   for (step = 0, i = 0; step < MANY_ROWS; step++, i = (i + 1237) % MANY_ROWS)
     {
        ck_assert_uint_eq(_property_uint_expected(many->rows[i], "self.height"), _many_height(i));
     }
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), sum);

   for (i = 0; i < MANY_ROWS; i++)
     {
        efl_unref(many->rows[i]);
     }
   free(many);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// The property_set takes a copy: the value given is the caller's to free.
static void
_uint_set(Efl_Model *model, const char *property, unsigned int value)
{
   Eina_Value *v = eina_value_uint_new(value);

   efl_future_then(model, efl_model_property_set(model, property, v));
   eina_value_free(v);
}

static void
_row_keep(void *data, const Efl_Event *ev)
{
   Efl_Model_Children_Event *cev = ev->info;
   Eo **kept = data;

   *kept = efl_ref(cev->child);
}

// Rows of every kind of size: those of the original index i keep _many_height(i).
static void
_many_heights_check(Many_Rows *many, unsigned int skip, unsigned int skip2)
{
   unsigned int i;

   for (i = 0; i < MANY_ROWS; i++)
     {
        if (i == skip || i == skip2) continue;
        ck_assert_uint_eq(_property_uint_expected(many->rows[i], "self.height"), _many_height(i));
        ck_assert_uint_eq(_property_uint_expected(many->rows[i], "self.width"),
                          1000 + _many_height(i));
     }
}

// A row inserted in front of the others, or removed from the front or from
// the middle, moves the rows after it: each keeps the size it was told to be,
// whichever block it was in and goes to, and the row that came in has none.
EFL_START_TEST(efl_ui_exact_model_insert_remove)
{
   Efl_Generic_Model *base_model;
   Efl_Model *model;
   Efl_Model_Children_Event ev = { 0 };
   Many_Rows *many = calloc(1, sizeof (Many_Rows));
   Eo *row, *added = NULL;
   unsigned int i, loops, sum = 0;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < MANY_ROWS; i++)
     {
        efl_model_child_add(base_model);
     }
   model = efl_add_ref(EFL_UI_EXACT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);

   eina_future_then(efl_model_children_slice_get(model, 0, MANY_ROWS), _many_rows_then, many, NULL);
   for (loops = 0; (loops < 50) && many->count < MANY_ROWS; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(many->count, MANY_ROWS);

   for (i = 0; i < MANY_ROWS; i++)
     {
        _uint_set(many->rows[i], "self.height", _many_height(i));
        _uint_set(many->rows[i], "self.width", 1000 + _many_height(i));
        sum += _many_height(i);
     }
   _many_heights_check(many, MANY_ROWS, MANY_ROWS);

   // In front of the others, as a model that can insert announces it
   row = efl_add(EFL_GENERIC_MODEL_CLASS, base_model);
   ev.index = 0;
   ev.child = row;
   efl_event_callback_add(model, EFL_MODEL_EVENT_CHILD_ADDED, _row_keep, &added);
   efl_event_callback_call(base_model, EFL_MODEL_EVENT_CHILD_ADDED, &ev);
   efl_event_callback_del(model, EFL_MODEL_EVENT_CHILD_ADDED, _row_keep, &added);
   ck_assert_ptr_ne(added, NULL);
   ck_assert_uint_eq(efl_composite_model_index_get(added), 0);
   ck_assert_int_eq(_property_error_expected(added, "self.height"), EAGAIN);
   ck_assert_int_eq(_property_error_expected(added, "self.width"), EAGAIN);
   _many_heights_check(many, MANY_ROWS, MANY_ROWS);
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), sum);

   // The row that came in is measured on its own
   _uint_set(added, "self.height", 5);
   ck_assert_uint_eq(_property_uint_expected(added, "self.height"), 5);
   _many_heights_check(many, MANY_ROWS, MANY_ROWS);

   // And leaves again, from the front
   ev.child = NULL;
   efl_event_callback_call(base_model, EFL_MODEL_EVENT_CHILD_REMOVED, &ev);
   _many_heights_check(many, MANY_ROWS, MANY_ROWS);
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), sum);

   // A row of the second block leaves: those after are one closer
   ck_assert_uint_eq(efl_composite_model_index_get(many->rows[1030]), 1030);
   ev.index = 1030;
   efl_event_callback_call(base_model, EFL_MODEL_EVENT_CHILD_REMOVED, &ev);
   _many_heights_check(many, 1030, MANY_ROWS);
   ck_assert_uint_eq(efl_composite_model_index_get(many->rows[1031]), 1030);
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), sum - _many_height(1030));

   // The first row leaves
   ev.index = 0;
   efl_event_callback_call(base_model, EFL_MODEL_EVENT_CHILD_REMOVED, &ev);
   _many_heights_check(many, 1030, 0);
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"),
                     sum - _many_height(1030) - _many_height(0));

   efl_unref(added);
   for (i = 0; i < MANY_ROWS; i++)
     {
        efl_unref(many->rows[i]);
     }
   free(many);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// Builds an Exact_Model over MANY_ROWS rows, with the rows of it.
static Efl_Model *
_many_exact_add(Efl_Generic_Model **base_model, Many_Rows *many)
{
   Efl_Model *model;
   unsigned int i, loops;

   *base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < MANY_ROWS; i++)
     {
        efl_model_child_add(*base_model);
     }
   model = efl_add_ref(EFL_UI_EXACT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, *base_model));
   ck_assert(!!model);

   many->count = 0;
   eina_future_then(efl_model_children_slice_get(model, 0, MANY_ROWS), _many_rows_then, many, NULL);
   for (loops = 0; (loops < 50) && (many->count < MANY_ROWS); loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(many->count, MANY_ROWS);
   return model;
}

// Width and height of the rows of the order, each its own value.
static void
_many_sizes_set(Many_Rows *many, const unsigned int *order, unsigned int count)
{
   unsigned int i;

   for (i = 0; i < count; i++)
     {
        _uint_set(many->rows[order[i]], "self.height", 10 + i);
        _uint_set(many->rows[order[i]], "self.width", 500 + i);
        ck_assert_uint_eq(_property_uint_expected(many->rows[order[i]], "self.height"), 10 + i);
        ck_assert_uint_eq(_property_uint_expected(many->rows[order[i]], "self.width"), 500 + i);
     }
}

static void
_many_sizes_check(Many_Rows *many, const unsigned int *order, unsigned int count)
{
   unsigned int i;

   for (i = 0; i < count; i++)
     {
        ck_assert_uint_eq(_property_uint_expected(many->rows[order[i]], "self.height"), 10 + i);
        ck_assert_uint_eq(_property_uint_expected(many->rows[order[i]], "self.width"), 500 + i);
     }
   // Rows nobody measured, in the blocks filled with zeros, are not ready
   ck_assert_int_eq(_property_error_expected(many->rows[5], "self.height"), EAGAIN);
   ck_assert_int_eq(_property_error_expected(many->rows[5], "self.width"), EAGAIN);
   ck_assert_int_eq(_property_error_expected(many->rows[1500], "self.height"), EAGAIN);
   ck_assert_int_eq(_property_error_expected(many->rows[1500], "self.width"), EAGAIN);
}

static void
_many_free(Many_Rows *many, Efl_Model *model, Efl_Generic_Model *base_model)
{
   unsigned int i;

   for (i = 0; i < MANY_ROWS; i++)
     {
        efl_unref(many->rows[i]);
     }
   efl_unref(model);
   efl_unref(base_model);
}

// A row that leaves takes its height out of what the average is over, and
// is no longer one that was seen.
EFL_START_TEST(efl_ui_average_model_remove)
{
   Efl_Model *base_model, *model;
   Many_Rows *many = calloc(1, sizeof (Many_Rows));
   Many_Rows *base_rows = calloc(1, sizeof (Many_Rows));
   unsigned int i, loops;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 4; i++)
     {
        efl_model_child_add(base_model);
     }
   model = efl_add_ref(EFL_UI_AVERAGE_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);
   eina_future_then(efl_model_children_slice_get(model, 0, 4), _many_rows_then, many, NULL);
   eina_future_then(efl_model_children_slice_get(base_model, 0, 4), _many_rows_then, base_rows, NULL);
   for (loops = 0; (loops < 50) && ((many->count < 4) || (base_rows->count < 4)); loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(many->count, 4);
   ck_assert_uint_eq(base_rows->count, 4);

   for (i = 0; i < 3; i++)
     {
        _uint_set(many->rows[i], "self.height", 10 * (i + 1));
     }
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   // 60 over 3 seen rows, for 4 rows
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 80);

   // One that was seen leaves: 30 over 2, for 3
   efl_model_child_del(base_model, base_rows->rows[2]);
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 45);

   // One that was not: 30 over 2, for 2
   efl_model_child_del(base_model, base_rows->rows[3]);
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 30);

   // A row that was seen is measured again: 60 over 2, for 2
   _uint_set(many->rows[1], "self.height", 50);
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 60);

   for (i = 0; i < 4; i++)
     {
        efl_unref(many->rows[i]);
        efl_unref(base_rows->rows[i]);
     }
   free(many);
   free(base_rows);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// A row that is measured again with a smaller size makes the average go
// down, and is not a row that wrapped the total around.
EFL_START_TEST(efl_ui_average_model_shrink)
{
   Efl_Model *base_model, *model;
   Many_Rows *many = calloc(1, sizeof (Many_Rows));
   unsigned int loops;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   efl_model_child_add(base_model);
   efl_model_child_add(base_model);
   efl_model_child_add(base_model);
   model = efl_add_ref(EFL_UI_AVERAGE_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);
   eina_future_then(efl_model_children_slice_get(model, 0, 3), _many_rows_then, many, NULL);
   for (loops = 0; (loops < 50) && many->count < 3; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(many->count, 3);

   // (the average is over the rows that were told, and is for all of them)
   _uint_set(many->rows[0], "self.height", 50);
   _uint_set(many->rows[1], "self.height", 10);
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 90);

   _uint_set(many->rows[0], "self.height", 30);
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 60);

   for (loops = 0; loops < 3; loops++)
     {
        efl_unref(many->rows[loops]);
     }
   free(many);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// A row that is measured and removed before the average heard of the
// measure is not one that is averaged over, when it finally does.
EFL_START_TEST(efl_ui_average_model_remove_pending)
{
   Efl_Model *base_model, *model;
   Many_Rows *many = calloc(1, sizeof (Many_Rows));
   Many_Rows *base_rows = calloc(1, sizeof (Many_Rows));
   unsigned int i, loops;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 4; i++)
     {
        efl_model_child_add(base_model);
     }
   model = efl_add_ref(EFL_UI_AVERAGE_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);
   eina_future_then(efl_model_children_slice_get(model, 0, 4), _many_rows_then, many, NULL);
   eina_future_then(efl_model_children_slice_get(base_model, 0, 4), _many_rows_then, base_rows, NULL);
   for (loops = 0; (loops < 50) && ((many->count < 4) || (base_rows->count < 4)); loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(many->count, 4);
   ck_assert_uint_eq(base_rows->count, 4);

   // Measured and removed in the same turn: nothing was ever averaged
   _uint_set(many->rows[0], "self.height", 40);
   efl_model_child_del(base_model, base_rows->rows[0]);
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 0);

   // And the ones that are there still do average
   _uint_set(many->rows[1], "self.height", 10);
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 30);

   // A row that was counted, measured again and removed in the same turn
   _uint_set(many->rows[1], "self.height", 25);
   efl_model_child_del(base_model, base_rows->rows[1]);
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 0);

   for (i = 0; i < 4; i++)
     {
        efl_unref(many->rows[i]);
        efl_unref(base_rows->rows[i]);
     }
   free(many);
   free(base_rows);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// A row that was told its height, and whose model is deleted before the
// loop gets to it, is not averaged into a model that is not there any more:
// it must not write to it, tell of it or edit what it kept (valgrind sees it,
// the rows being held by the test as they are by their updates).
EFL_START_TEST(efl_ui_average_model_root_deleted)
{
   Efl_Model *base_model, *model;
   Many_Rows *many = calloc(1, sizeof (Many_Rows));
   unsigned int i, loops;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 4; i++)
     {
        efl_model_child_add(base_model);
     }
   model = efl_add_ref(EFL_UI_AVERAGE_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);
   eina_future_then(efl_model_children_slice_get(model, 0, 4), _many_rows_then, many, NULL);
   for (loops = 0; (loops < 50) && many->count < 4; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(many->count, 4);

   _uint_set(many->rows[0], "self.height", 40);
   _uint_set(many->rows[1], "self.height", 10);
   // The model goes in the same turn, and the rows are still held
   efl_del(model);
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   // The two that were told are not averaged into what is gone
   for (i = 0; i < 4; i++)
     {
        efl_unref(many->rows[i]);
     }
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }

   free(many);
   efl_unref(base_model);
}
EFL_END_TEST

// The last ref to a row that was told its height is dropped in the same turn:
// the ref the pending update holds is then the last one, and it must not be
// let go of while the future that holds the update is being freed (valgrind
// sees the row, and the future with it, being destroyed from inside it).
EFL_START_TEST(efl_ui_average_model_last_ref_dropped)
{
   Efl_Model *base_model, *model;
   Many_Rows *many = calloc(1, sizeof (Many_Rows));
   unsigned int i, loops;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 4; i++)
     {
        efl_model_child_add(base_model);
     }
   model = efl_add_ref(EFL_UI_AVERAGE_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);
   eina_future_then(efl_model_children_slice_get(model, 0, 4), _many_rows_then, many, NULL);
   for (loops = 0; (loops < 50) && many->count < 4; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(many->count, 4);

   _uint_set(many->rows[0], "self.height", 40);
   _uint_set(many->rows[1], "self.height", 10);
   // The user lets go in the same turn: only the updates hold the rows now
   for (i = 0; i < 4; i++)
     {
        efl_unref(many->rows[i]);
     }
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 100);

   free(many);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

static void
_total_height_changed(void *data, const Efl_Event *ev)
{
   Efl_Model_Property_Event *pev = ev->info;
   unsigned int *count = data;
   Eina_Array_Iterator it;
   const char *property;
   unsigned int i;

   if (!pev || !pev->changed_properties) return;
   EINA_ARRAY_ITER_NEXT(pev->changed_properties, i, property, it)
     if (eina_streq(property, "total.height")) (*count)++;
}

// The first row that is measured changes what the rows nobody measured are
// estimated to be (from a stand-in to its height), even when that height is 0
// and the average, a number, stays what it was: it is told of like any change.
EFL_START_TEST(efl_ui_average_model_first_measure)
{
   Efl_Model *base_model, *model;
   Many_Rows *many = calloc(1, sizeof (Many_Rows));
   unsigned int i, loops, changes = 0, known_changes;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 4; i++)
     {
        efl_model_child_add(base_model);
     }
   model = efl_add_ref(EFL_UI_AVERAGE_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);
   efl_event_callback_add(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _total_height_changed, &changes);
   eina_future_then(efl_model_children_slice_get(model, 0, 4), _many_rows_then, many, NULL);
   for (loops = 0; (loops < 50) && many->count < 4; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(many->count, 4);
   changes = 0;

   _uint_set(many->rows[0], "self.height", 0);
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   known_changes = changes;
   ck_assert_uint_eq(known_changes, 1);

   // (and from then on, the average of 0 staying 0 is no change)
   _uint_set(many->rows[1], "self.height", 0);
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(changes, known_changes);

   for (i = 0; i < 4; i++)
     {
        efl_unref(many->rows[i]);
     }
   free(many);
   efl_event_callback_del(model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _total_height_changed, &changes);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// A row that was removed has no place in the model any more: what is told
// to it is not for the row that is now at the index it had.
EFL_START_TEST(efl_ui_exact_model_removed_row_write)
{
   Efl_Generic_Model *base_model;
   Efl_Model *model;
   Efl_Model_Children_Event ev = { 0 };
   Many_Rows *many = calloc(1, sizeof (Many_Rows));
   Eina_Value told = eina_value_uint_init(500);
   Eina_Error err = -1;
   unsigned int i, loops;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 10; i++)
     {
        efl_model_child_add(base_model);
     }
   model = efl_add_ref(EFL_UI_EXACT_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);

   eina_future_then(efl_model_children_slice_get(model, 0, 10), _many_rows_then, many, NULL);
   for (loops = 0; (loops < 50) && many->count < 10; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(many->count, 10);

   for (i = 0; i < 10; i++)
     {
        _uint_set(many->rows[i], "self.height", 10 + i);
     }
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 145);

   ev.index = 5;
   efl_event_callback_call(base_model, EFL_MODEL_EVENT_CHILD_REMOVED, &ev);
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 130);
   ck_assert_uint_eq(efl_composite_model_index_get(many->rows[5]), 0xFFFFFFFF);
   ck_assert_uint_eq(efl_composite_model_index_get(many->rows[6]), 5);

   // The row that left is told a size
   eina_future_then(efl_model_property_set(many->rows[5], "self.height", &told),
                    _gone_set_then, &err, NULL);
   for (loops = 0; (loops < 8) && err == -1; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(err, EFL_MODEL_ERROR_INCORRECT_VALUE);
   err = -1;
   eina_future_then(efl_model_property_set(many->rows[5], "self.width", &told),
                    _gone_set_then, &err, NULL);
   for (loops = 0; (loops < 8) && err == -1; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(err, EFL_MODEL_ERROR_INCORRECT_VALUE);

   // The row that took its place, and the total, are not touched
   ck_assert_uint_eq(_property_uint_expected(many->rows[6], "self.height"), 16);
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 130);
   ck_assert_int_eq(_property_error_expected(many->rows[5], "self.height"), EAGAIN);

   for (i = 0; i < 10; i++)
     {
        efl_unref(many->rows[i]);
     }
   eina_value_flush(&told);
   free(many);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// What a shift of the sizes must give, kept on the side: the row now at each
// index, and the height it was told (0 for none).
typedef struct _Shift_Rows Shift_Rows;
struct _Shift_Rows
{
   Efl_Model *base;
   Efl_Model *model;
   Eo **cur;
   unsigned int *exp;
   unsigned int n;
};

static void
_shift_check(Shift_Rows *sr)
{
   unsigned int i, sum = 0;

   for (i = 0; i < sr->n; i++)
     {
        ck_assert_uint_eq(efl_composite_model_index_get(sr->cur[i]), i);
        if (!sr->exp[i])
          ck_assert_int_eq(_property_error_expected(sr->cur[i], "self.height"), EAGAIN);
        else
          ck_assert_uint_eq(_property_uint_expected(sr->cur[i], "self.height"), sr->exp[i]);
        sum += sr->exp[i];
     }
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.height"), sum);
}

static void
_shift_height(Shift_Rows *sr, unsigned int at, unsigned int h)
{
   _uint_set(sr->cur[at], "self.height", h);
   sr->exp[at] = h;
}

static void
_shift_insert(Shift_Rows *sr, unsigned int at, unsigned int h)
{
   Efl_Model_Children_Event ev = { 0 };
   Eo *row = efl_add(EFL_GENERIC_MODEL_CLASS, sr->base);
   Eo *added = NULL;

   ev.index = at;
   ev.child = row;
   efl_event_callback_add(sr->model, EFL_MODEL_EVENT_CHILD_ADDED, _row_keep, &added);
   efl_event_callback_call(sr->base, EFL_MODEL_EVENT_CHILD_ADDED, &ev);
   efl_event_callback_del(sr->model, EFL_MODEL_EVENT_CHILD_ADDED, _row_keep, &added);
   ck_assert_ptr_ne(added, NULL);
   memmove(sr->cur + at + 1, sr->cur + at, (sr->n - at) * sizeof (Eo *));
   memmove(sr->exp + at + 1, sr->exp + at, (sr->n - at) * sizeof (unsigned int));
   sr->cur[at] = added;
   sr->exp[at] = 0;
   sr->n++;
   if (h) _shift_height(sr, at, h);
}

static void
_shift_remove(Shift_Rows *sr, unsigned int at)
{
   Efl_Model_Children_Event ev = { 0 };

   ev.index = at;
   efl_event_callback_call(sr->base, EFL_MODEL_EVENT_CHILD_REMOVED, &ev);
   efl_unref(sr->cur[at]);
   memmove(sr->cur + at, sr->cur + at + 1, (sr->n - at - 1) * sizeof (Eo *));
   memmove(sr->exp + at, sr->exp + at + 1, (sr->n - at - 1) * sizeof (unsigned int));
   sr->n--;
}

static Shift_Rows *
_shift_new(unsigned int n)
{
   Shift_Rows *sr = calloc(1, sizeof (Shift_Rows));
   Many_Rows *many = calloc(1, sizeof (Many_Rows));
   unsigned int i, loops;

   ck_assert_uint_le(n, MANY_ROWS);
   sr->cur = calloc(n + 16, sizeof (Eo *));
   sr->exp = calloc(n + 16, sizeof (unsigned int));
   sr->base = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < n; i++)
     {
        efl_model_child_add(sr->base);
     }
   sr->model = efl_add_ref(EFL_UI_EXACT_MODEL_CLASS, efl_main_loop_get(),
                           efl_ui_view_model_set(efl_added, sr->base));
   ck_assert(!!sr->model);
   eina_future_then(efl_model_children_slice_get(sr->model, 0, n), _many_rows_then, many, NULL);
   // (the slice puts what it gets in the array of Many_Rows, which holds MANY_ROWS)
   for (loops = 0; (loops < 100) && many->count < n; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(many->count, n);
   for (i = 0; i < n; i++)
     {
        sr->cur[i] = many->rows[i];
     }
   sr->n = n;
   free(many);
   return sr;
}

static void
_shift_free(Shift_Rows *sr)
{
   unsigned int i;

   for (i = 0; i < sr->n; i++)
     {
        efl_unref(sr->cur[i]);
     }
   free(sr->cur);
   free(sr->exp);
   efl_unref(sr->model);
   efl_unref(sr->base);
   free(sr);
}

// Most rows were never measured: the blocks of zeros between those that were
// are not touched by a shift, and what is on each side of them must still
// move, and on the boundary of two blocks too.
EFL_START_TEST(efl_ui_exact_model_shift_unmeasured)
{
   Shift_Rows *sr = _shift_new(3400);
   unsigned int i;

   for (i = 0; i < 10; i++)
     {
        _shift_height(sr, i, 3 + i);
     }
   for (i = 2600; i < 2610; i++)
     {
        _shift_height(sr, i, 100 + i % 50);
     }
   for (i = 3390; i < 3400; i++)
     {
        _shift_height(sr, i, 200 + i % 50);
     }
   _shift_check(sr);

   _shift_insert(sr, 5, 0);
   _shift_check(sr);
   // The last row of the first block, and the first of the second
   _shift_insert(sr, 1023, 0);
   _shift_check(sr);
   _shift_insert(sr, 1024, 77);
   _shift_check(sr);
   _shift_remove(sr, 1024);
   _shift_check(sr);
   _shift_remove(sr, 1023);
   _shift_check(sr);
   // The sizes of the second block are of rows that never were measured
   _shift_remove(sr, 1500);
   _shift_check(sr);
   _shift_insert(sr, 2047, 31);
   _shift_check(sr);
   _shift_insert(sr, 2048, 0);
   _shift_check(sr);
   _shift_remove(sr, 0);
   _shift_check(sr);
   // At the very end, and the one that was the last
   _shift_insert(sr, sr->n, 12);
   _shift_check(sr);
   _shift_remove(sr, sr->n - 1);
   _shift_check(sr);
   _shift_remove(sr, sr->n - 1);
   _shift_check(sr);
   _shift_free(sr);
}
EFL_END_TEST

static unsigned int _cri_count;

static void
_cri_log(const Eina_Log_Domain *d EINA_UNUSED, Eina_Log_Level level,
         const char *file EINA_UNUSED, const char *fnc EINA_UNUSED, int line EINA_UNUSED,
         const char *fmt EINA_UNUSED, void *data EINA_UNUSED, va_list args EINA_UNUSED)
{
   if (level <= EINA_LOG_LEVEL_CRITICAL)
     {
        _cri_count++;
     }
}

// A row of the model that goes is no longer a view on the row of its source.
// That row belongs to the source model, and nothing else holds it: letting
// go of it must not delete it (which efl_unref() does, with a CRI).
EFL_START_TEST(efl_ui_composite_model_source_release)
{
   Shift_Rows *sr = _shift_new(8);

   _cri_count = 0;
   eina_log_print_cb_set(_cri_log, NULL);
   _shift_insert(sr, 3, 0);
   _shift_remove(sr, 3);
   _shift_remove(sr, 0);
   _shift_free(sr);
   eina_log_print_cb_set(eina_log_print_cb_stderr, NULL);

   ck_assert_uint_eq(_cri_count, 0);
}
EFL_END_TEST

// The total width is the one of the widest row, and goes down with it.
EFL_START_TEST(efl_ui_exact_model_remove_widest)
{
   Shift_Rows *sr = _shift_new(1100);
   Eo *r[4];
   unsigned int w[4] = { 20, 80, 90, 20 };
   unsigned int at[4] = { 5, 3, 1030, 7 };
   unsigned int i;

   for (i = 0; i < 4; i++)
     {
        r[i] = sr->cur[at[i]];
        _uint_set(r[i], "self.width", w[i]);
     }
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.width"), 90);

   // Not the widest: nothing changes
   _shift_remove(sr, 5);
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.width"), 90);
   // The widest, in another block than the others
   _shift_remove(sr, 1029);
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.width"), 80);
   _shift_remove(sr, 3);
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.width"), 20);
   ck_assert_uint_eq(efl_composite_model_index_get(r[3]), 5);
   _shift_remove(sr, 5);
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.width"), 0);
   _shift_free(sr);
}
EFL_END_TEST

// The widest row gets narrower while another widest one is only in a slot
// that was not stored: the total is the width of that one.
EFL_START_TEST(efl_ui_exact_model_narrow_widest_other_slot)
{
   Shift_Rows *sr = _shift_new(1100);

   _uint_set(sr->cur[0], "self.width", 300);
   _uint_set(sr->cur[1050], "self.width", 300);
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.width"), 300);
   _uint_set(sr->cur[0], "self.width", 100);
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.width"), 300);
   _uint_set(sr->cur[1050], "self.width", 50);
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.width"), 100);
   _shift_free(sr);
}
EFL_END_TEST

// What cannot be stored when the sizes move is lost, and no row is then
// said to have a size that is not its own: every size is forgotten, and the
// totals with them.
EFL_START_TEST(efl_ui_exact_model_shift_store_fails)
{
   Shift_Rows *sr = _shift_new(1100);
   Eo *r;

   _shift_height(sr, 5, 10);
   _shift_height(sr, 1030, 20);
   _uint_set(sr->cur[5], "self.width", 7);
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.width"), 7);

   setenv("EFL_UI_EXACT_MODEL_STORE_FAIL", "1", 1);
   _shift_remove(sr, 3);
   unsetenv("EFL_UI_EXACT_MODEL_STORE_FAIL");
   memset(sr->exp, 0, sr->n * sizeof (unsigned int));
   _shift_check(sr);
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.width"), 0);

   // The model goes on from what it has: what is told now is kept
   r = sr->cur[1030];
   _shift_height(sr, 1030, 33);
   _shift_check(sr);
   ck_assert_uint_eq(efl_composite_model_index_get(r), 1030);
   _shift_remove(sr, 3);
   _shift_check(sr);
   _shift_free(sr);
}
EFL_END_TEST

// The average over what was told is over nothing when the sizes were forgotten.
EFL_START_TEST(efl_ui_average_model_forgotten)
{
   Efl_Model *base_model, *model;
   Many_Rows *many = calloc(1, sizeof (Many_Rows));
   Many_Rows *base_rows = calloc(1, sizeof (Many_Rows));
   unsigned int i, loops;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 4; i++)
     {
        efl_model_child_add(base_model);
     }
   model = efl_add_ref(EFL_UI_AVERAGE_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);
   eina_future_then(efl_model_children_slice_get(model, 0, 4), _many_rows_then, many, NULL);
   eina_future_then(efl_model_children_slice_get(base_model, 0, 4), _many_rows_then, base_rows, NULL);
   for (loops = 0; (loops < 50) && ((many->count < 4) || (base_rows->count < 4)); loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(many->count, 4);

   for (i = 0; i < 3; i++)
     {
        _uint_set(many->rows[i], "self.height", 10 * (i + 1));
     }
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   // 60 over 3 seen rows, for 4 rows
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 80);

   // The last row leaves, and the sizes cannot be moved
   setenv("EFL_UI_EXACT_MODEL_STORE_FAIL", "1", 1);
   efl_model_child_del(base_model, base_rows->rows[3]);
   unsetenv("EFL_UI_EXACT_MODEL_STORE_FAIL");
   ck_assert_int_eq(_property_error_expected(many->rows[0], "self.height"), EAGAIN);
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 0);

   // What is told after is what the average is over
   _uint_set(many->rows[0], "self.height", 50);
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 150);

   for (i = 0; i < 4; i++)
     {
        efl_unref(many->rows[i]);
        efl_unref(base_rows->rows[i]);
     }
   free(many);
   free(base_rows);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// Rows that are all as wide: the total width is theirs as long as one is left.
EFL_START_TEST(efl_ui_exact_model_remove_widest_alike)
{
   Shift_Rows *sr = _shift_new(2100);
   unsigned int i;

   for (i = 0; i < 2100; i += 3)
     _uint_set(sr->cur[i], "self.width", 60);
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.width"), 60);
   // (every third row is measured: the last of them is the row 2097)
   for (i = 0; i < 2100; i++)
     {
        _shift_remove(sr, 0);
        ck_assert_uint_eq(_property_uint_expected(sr->model, "total.width"),
                          i < 2097 ? 60 : 0);
     }
   _shift_free(sr);
}
EFL_END_TEST

// A listener that runs before all the others (the priority of the Exact_Model's
// own is lower than any other) is told of a removal after the sizes moved, and
// the row that left is still the row it was until the event is over.
typedef struct _Removal_Watch Removal_Watch;
struct _Removal_Watch
{
   Efl_Model *model;
   Eo *row;
   unsigned int total_height;
   unsigned int index;
   Eina_Bool told;
};

static void
_removal_watch(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Removal_Watch *rw = data;

   rw->total_height = _property_uint_expected(rw->model, "total.height");
   rw->index = efl_composite_model_index_get(rw->row);
   rw->told = EINA_TRUE;
}

EFL_START_TEST(efl_ui_exact_model_remove_event_state)
{
   Shift_Rows *sr = _shift_new(10);
   Removal_Watch rw = { 0 };
   unsigned int i;

   for (i = 0; i < 10; i++)
     {
        _shift_height(sr, i, 10 + i);
     }
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.height"), 145);

   rw.model = sr->model;
   rw.row = efl_ref(sr->cur[5]);
   efl_event_callback_priority_add(sr->model, EFL_MODEL_EVENT_CHILD_REMOVED,
                                   EFL_CALLBACK_PRIORITY_BEFORE - 1, _removal_watch, &rw);
   _shift_remove(sr, 5);
   efl_event_callback_del(sr->model, EFL_MODEL_EVENT_CHILD_REMOVED, _removal_watch, &rw);

   ck_assert(rw.told);
   // The total is already without the row, and the row is still the 5th
   ck_assert_uint_eq(rw.total_height, 130);
   ck_assert_uint_eq(rw.index, 5);
   ck_assert_uint_eq(efl_composite_model_index_get(rw.row), 0xFFFFFFFF);
   ck_assert_uint_eq(efl_composite_model_index_get(sr->cur[5]), 5);
   ck_assert_uint_eq(_property_uint_expected(sr->cur[5], "self.height"), 16);

   efl_unref(rw.row);
   _shift_free(sr);
}
EFL_END_TEST

// Blocks that are all measured: what falls out of the last block as the rows
// move is a size too, and has a block of its own.
EFL_START_TEST(efl_ui_exact_model_shift_overflow)
{
   Shift_Rows *sr = _shift_new(2048);
   unsigned int i;

   for (i = 0; i < 2048; i++)
     {
        _shift_height(sr, i, 1 + (i * 7) % 97);
     }
   _shift_check(sr);

   _shift_insert(sr, 1023, 0);
   _shift_check(sr);
   _shift_insert(sr, 1024, 0);
   _shift_check(sr);
   _shift_insert(sr, 0, 9);
   _shift_check(sr);
   // Again over the block that was made for it
   for (i = 0; i < 4; i++)
     {
        _shift_insert(sr, 1024, 2 + i);
        _shift_check(sr);
     }
   _shift_remove(sr, 1024);
   _shift_check(sr);
   _shift_remove(sr, 1023);
   _shift_check(sr);
   _shift_remove(sr, sr->n - 1);
   _shift_check(sr);
   for (i = 0; i < 6; i++)
     {
        _shift_remove(sr, 3);
        _shift_check(sr);
     }
   _shift_free(sr);
}
EFL_END_TEST

// Exact_Models that held sizes in several blocks are let go of: what they kept
// compressed (both lists), the buffers of their slots and the block of zeros
// they fill the gaps with are theirs to free (valgrind sees what is not). The
// last block is written first so that the blocks before it are filled with
// zeros, and each of the first ones is visited again after the slots were
// reused. The block of zeros is shared: it stays while another model holds it
// and is made again once the last one is gone, and what a model reads is
// the same throughout.
EFL_START_TEST(efl_ui_exact_model_destroyed)
{
   Efl_Generic_Model *base_a, *base_b, *base_c;
   Efl_Model *a, *b, *c;
   Many_Rows *many_a = calloc(1, sizeof (Many_Rows));
   Many_Rows *many_b = calloc(1, sizeof (Many_Rows));
   Many_Rows *many_c = calloc(1, sizeof (Many_Rows));
   static const unsigned int order[] = { 3400, 2100, 1100, 50, 3401, 2101, 1101, 51, 3402 };

   a = _many_exact_add(&base_a, many_a);
   b = _many_exact_add(&base_b, many_b);
   _many_sizes_set(many_a, order, EINA_C_ARRAY_LENGTH(order));
   _many_sizes_set(many_b, order, EINA_C_ARRAY_LENGTH(order));
   _many_sizes_check(many_a, order, EINA_C_ARRAY_LENGTH(order));

   // The first one goes: the second still reads what it was told, and the gaps
   _many_free(many_a, a, base_a);
   free(many_a);
   _many_sizes_check(many_b, order, EINA_C_ARRAY_LENGTH(order));

   // The last one goes and the block of zeros with it: a new one makes it again
   _many_free(many_b, b, base_b);
   free(many_b);
   c = _many_exact_add(&base_c, many_c);
   _many_sizes_set(many_c, order, EINA_C_ARRAY_LENGTH(order));
   _many_sizes_check(many_c, order, EINA_C_ARRAY_LENGTH(order));
   _many_free(many_c, c, base_c);
   free(many_c);
}
EFL_END_TEST

// A row is told its height when its block has no slot, and the slot that is
// given up cannot be stored: every size is forgotten, and the row is the one
// that is measured after that. The average is over that row alone.
EFL_START_TEST(efl_ui_average_model_forgotten_in_set)
{
   Efl_Model *base_model, *model;
   Many_Rows *many = calloc(1, sizeof (Many_Rows));
   unsigned int i, loops;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < MANY_ROWS; i++)
     {
        efl_model_child_add(base_model);
     }
   model = efl_add_ref(EFL_UI_AVERAGE_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);
   eina_future_then(efl_model_children_slice_get(model, 0, MANY_ROWS), _many_rows_then, many, NULL);
   for (loops = 0; (loops < 50) && many->count < MANY_ROWS; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(many->count, MANY_ROWS);

   // The three slots hold a block each, written to
   for (i = 0; i < 3; i++)
     {
        _uint_set(many->rows[i * 1024], "self.height", 10 * (i + 1));
     }
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 20 * MANY_ROWS);

   setenv("EFL_UI_EXACT_MODEL_STORE_FAIL", "1", 1);
   _uint_set(many->rows[3072], "self.height", 40);
   unsetenv("EFL_UI_EXACT_MODEL_STORE_FAIL");
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_int_eq(_property_error_expected(many->rows[0], "self.height"), EAGAIN);
   ck_assert_uint_eq(_property_uint_expected(many->rows[3072], "self.height"), 40);
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 40 * MANY_ROWS);

   for (i = 0; i < MANY_ROWS; i++)
     {
        efl_unref(many->rows[i]);
     }
   free(many);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

typedef struct _Nested_Removal Nested_Removal;
struct _Nested_Removal
{
   Efl_Model *base;
   Eo *then;
   Eina_Bool done;
};

// (listens before the average does)
static void
_nested_removal(void *data, const Efl_Event *ev EINA_UNUSED)
{
   Nested_Removal *nr = data;

   if (nr->done) return;
   nr->done = EINA_TRUE;
   efl_model_child_del(nr->base, nr->then);
}

// A listener that comes before the Average_Model removes another row while the
// first is told of: what the first one had been told is still taken out.
EFL_START_TEST(efl_ui_average_model_remove_nested)
{
   Efl_Model *base_model, *model;
   Many_Rows *many = calloc(1, sizeof (Many_Rows));
   Many_Rows *base_rows = calloc(1, sizeof (Many_Rows));
   Nested_Removal nr = { 0 };
   unsigned int i, loops;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 4; i++)
     {
        efl_model_child_add(base_model);
     }
   model = efl_add_ref(EFL_UI_AVERAGE_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);
   eina_future_then(efl_model_children_slice_get(model, 0, 4), _many_rows_then, many, NULL);
   eina_future_then(efl_model_children_slice_get(base_model, 0, 4), _many_rows_then, base_rows, NULL);
   for (loops = 0; (loops < 50) && (many->count < 4 || base_rows->count < 4); loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(many->count, 4);
   ck_assert_uint_eq(base_rows->count, 4);

   _uint_set(many->rows[0], "self.height", 10);
   _uint_set(many->rows[1], "self.height", 20);
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 60);

   nr.base = base_model;
   nr.then = base_rows->rows[3];
   efl_event_callback_priority_add(model, EFL_MODEL_EVENT_CHILD_REMOVED, -150, _nested_removal, &nr);
   efl_model_child_del(base_model, base_rows->rows[0]);
   ck_assert(nr.done);
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   // The one told 20 is the only one left that was, for the two that are
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 40);
   efl_event_callback_del(model, EFL_MODEL_EVENT_CHILD_REMOVED, _nested_removal, &nr);

   for (i = 0; i < 4; i++)
     {
        efl_unref(many->rows[i]);
        efl_unref(base_rows->rows[i]);
     }
   free(many);
   free(base_rows);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

static void
_stop_removal(void *data, const Efl_Event *ev)
{
   Nested_Removal *nr = data;

   if (nr->done) return;
   nr->done = EINA_TRUE;
   efl_event_callback_stop(ev->object);
}

// A listener stops the removal of a row that was measured before the Average_Model is told of
// it (and what the Exact_Model kept of that row is never let go of by the end of the event). The
// removal of a row that was not measured, that comes next from the same place (the event is
// at the same address), does not take the height of the one before out of the average.
EFL_START_TEST(efl_ui_average_model_remove_after_stopped)
{
   Efl_Model *base_model, *model;
   Many_Rows *many = calloc(1, sizeof (Many_Rows));
   Many_Rows *base_rows = calloc(1, sizeof (Many_Rows));
   Nested_Removal nr = { 0 };
   unsigned int i, loops;

   base_model = efl_add_ref(EFL_GENERIC_MODEL_CLASS, efl_main_loop_get());
   for (i = 0; i < 4; i++)
     {
        efl_model_child_add(base_model);
     }
   model = efl_add_ref(EFL_UI_AVERAGE_MODEL_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);
   eina_future_then(efl_model_children_slice_get(model, 0, 4), _many_rows_then, many, NULL);
   eina_future_then(efl_model_children_slice_get(base_model, 0, 4), _many_rows_then, base_rows, NULL);
   for (loops = 0; (loops < 50) && (many->count < 4 || base_rows->count < 4); loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(many->count, 4);
   ck_assert_uint_eq(base_rows->count, 4);

   _uint_set(many->rows[0], "self.height", 10);
   _uint_set(many->rows[1], "self.height", 20);
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 60);

   efl_event_callback_priority_add(model, EFL_MODEL_EVENT_CHILD_REMOVED, -150, _stop_removal, &nr);
   efl_model_child_del(base_model, base_rows->rows[0]);
   ck_assert(nr.done);
   efl_event_callback_del(model, EFL_MODEL_EVENT_CHILD_REMOVED, _stop_removal, &nr);

   // (the Average_Model was not told of that one: it still has the 30 over 2 seen rows)
   efl_model_child_del(base_model, base_rows->rows[3]);
   for (loops = 0; loops < 8; loops++)
     {
        ecore_main_loop_iterate();
     }
   // The row that was not measured took nothing out: 30 over 2, for the 2 rows that are left
   ck_assert_uint_eq(_property_uint_expected(model, "total.height"), 30);

   for (i = 0; i < 4; i++)
     {
        efl_unref(many->rows[i]);
        efl_unref(base_rows->rows[i]);
     }
   free(many);
   free(base_rows);
   efl_unref(model);
   efl_unref(base_model);
}
EFL_END_TEST

// The widest row gets narrower: the total width is the widest that is left, as when it
// is removed.
EFL_START_TEST(efl_ui_exact_model_narrower_widest)
{
   Shift_Rows *sr = _shift_new(2100);

   _uint_set(sr->cur[3], "self.width", 60);
   _uint_set(sr->cur[1500], "self.width", 100);
   _uint_set(sr->cur[2000], "self.width", 100);
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.width"), 100);
   // another one is as wide: the total is the same
   _uint_set(sr->cur[1500], "self.width", 70);
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.width"), 100);
   // the last of them
   _uint_set(sr->cur[2000], "self.width", 20);
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.width"), 70);
   // not the widest: nothing to look for
   _uint_set(sr->cur[3], "self.width", 10);
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.width"), 70);
   _uint_set(sr->cur[1500], "self.width", 5);
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.width"), 20);
   // and wider again
   _uint_set(sr->cur[3], "self.width", 90);
   ck_assert_uint_eq(_property_uint_expected(sr->model, "total.width"), 90);
   _shift_free(sr);
}
EFL_END_TEST

void
efl_ui_model(TCase *tc)
{
   // This is done here as some system can not initialize computed value from another data at compile time.
   expected_average[0] =  base_ints[0] * 3 * 3;
   expected_average[1] = ((base_ints[0] * 3 + base_ints[1] * 3) * 3) / 2;
   expected_average[2] = base_ints[0] * 3 + base_ints[1] * 3 + base_ints[2] * 3;

   tcase_add_test(tc, efl_ui_homogeneous_model_test);
   tcase_add_test(tc, efl_ui_average_model_test);
   tcase_add_test(tc, efl_ui_average_model_remove);
   tcase_add_test(tc, efl_ui_average_model_shrink);
   tcase_add_test(tc, efl_ui_average_model_remove_pending);
   tcase_add_test(tc, efl_ui_average_model_root_deleted);
   tcase_add_test(tc, efl_ui_average_model_last_ref_dropped);
   tcase_add_test(tc, efl_ui_average_model_first_measure);
   tcase_add_test(tc, efl_ui_exact_model_test);
   tcase_add_test(tc, efl_ui_exact_model_gone_row);
   tcase_add_test(tc, efl_ui_exact_model_many_rows);
   tcase_add_test(tc, efl_ui_exact_model_insert_remove);
   tcase_add_test(tc, efl_ui_exact_model_removed_row_write);
   tcase_add_test(tc, efl_ui_exact_model_shift_unmeasured);
   tcase_add_test(tc, efl_ui_exact_model_shift_overflow);
   tcase_add_test(tc, efl_ui_exact_model_remove_event_state);
   tcase_add_test(tc, efl_ui_exact_model_shift_store_fails);
   tcase_add_test(tc, efl_ui_average_model_forgotten);
   tcase_add_test(tc, efl_ui_exact_model_remove_widest);
   tcase_add_test(tc, efl_ui_exact_model_remove_widest_alike);
   tcase_add_test(tc, efl_ui_exact_model_destroyed);
   tcase_add_test(tc, efl_ui_average_model_forgotten_in_set);
   tcase_add_test(tc, efl_ui_average_model_remove_nested);
   tcase_add_test(tc, efl_ui_average_model_remove_after_stopped);
   tcase_add_test(tc, efl_ui_exact_model_narrow_widest_other_slot);
   tcase_add_test(tc, efl_ui_exact_model_narrower_widest);
   tcase_add_test(tc, efl_ui_composite_model_source_release);
}
