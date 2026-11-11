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
   fail_if(real__ != 0);

   ck_assert_int_eq(_property_uint_expected(model, "total.width"), base_ints[0] * 2);
   ck_assert_int_eq(_property_uint_expected(model, "total.height"), base_ints[0] * 3 * 3);
   ck_assert_int_eq(_property_uint_expected(model, "item.width"), base_ints[0] * 2);
   ck_assert_int_eq(_property_uint_expected(model, "item.height"), base_ints[0] * 3);
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
        ck_assert_int_eq(_property_uint_expected(child, "self.width"), 0);
        ck_assert_int_eq(_property_uint_expected(child, "self.height"), 0);
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
   fail_if(real__ != 0);

   ck_assert_int_eq(_property_uint_expected(model, "total.width"), base_ints[2] * 2);
   ck_assert_int_eq(_property_uint_expected(model, "total.height"), base_ints[0] * 3 + base_ints[1] * 3 + base_ints[2] * 3);
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
        ck_assert_int_eq(_property_uint_expected(child, "self.width"), 0);
        ck_assert_int_eq(_property_uint_expected(child, "self.height"), 0);
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
   fail_if(real__ != 0);

   ck_assert_int_eq(_property_uint_expected(model, "total.width"), base_ints[2] * 2);
   ck_assert_int_eq(_property_uint_expected(model, "total.height"), base_ints[0] * 3 + base_ints[1] * 3 + base_ints[2] * 3);
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

// An Exact_Model that held sizes in several blocks is let go of: what it kept
// compressed, the buffers of its slots and the block of zeros it fills the gaps
// with are its to free (there is nothing to check here but valgrind). The last
// block is written first so that the blocks before it are filled with zeros,
// and each of the first ones is visited again after the slots were reused.
EFL_START_TEST(efl_ui_exact_model_destroyed)
{
   Efl_Generic_Model *base_model;
   Efl_Model *model;
   Many_Rows *many = calloc(1, sizeof (Many_Rows));
   static const unsigned int order[] = { 3400, 2100, 1100, 50, 3401, 2101, 1101, 51, 3402 };
   unsigned int i, loops;

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

   for (i = 0; i < EINA_C_ARRAY_LENGTH(order); i++)
     {
        Eina_Future *f = efl_model_property_set(many->rows[order[i]], "self.height",
                                                eina_value_uint_new(10 + i));

        efl_future_then(many->rows[order[i]], f);
        ck_assert_uint_eq(_property_uint_expected(many->rows[order[i]], "self.height"), 10 + i);
     }

   for (i = 0; i < MANY_ROWS; i++)
     {
        efl_unref(many->rows[i]);
     }
   free(many);
   efl_unref(model);
   efl_unref(base_model);
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
   tcase_add_test(tc, efl_ui_exact_model_test);
   tcase_add_test(tc, efl_ui_exact_model_many_rows);
   tcase_add_test(tc, efl_ui_exact_model_destroyed);
}
