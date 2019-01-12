#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "efl_ui_suite.h"
#include "efl_ui_model_homogeneous.eo.h"

static const int child_number = 3;
static const int base_ints[] = { 41, 42, 43 };

static Efl_Model *
_generate_base_model(void)
{
   Efl_Model_Item *base_model, *child;
   Eina_Value v;
   int i;

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

EFL_START_TEST(efl_ui_model_homogeneous_test)
{
   Efl_Model_Item *base_model, *model;
   Eina_Value *ret__;
   Eina_Future *future;
   int real__;

   base_model = _generate_base_model();

   model = efl_add_ref(EFL_UI_MODEL_HOMOGENEOUS_CLASS, efl_main_loop_get(),
                       efl_ui_view_model_set(efl_added, base_model));
   ck_assert(!!model);

   future = efl_model_children_slice_get(model, 0, efl_model_children_count_get(model));
   efl_future_then(model, future, .success = _children_homogeneous_slice_get_then);

   ck_assert_int_eq(_property_error_expected(model, "total.width"), EAGAIN);
   ck_assert_int_eq(_property_error_expected(model, "total.height"), EAGAIN);
   ck_assert_int_eq(_property_error_expected(model, "item.width"), EAGAIN);
   ck_assert_int_eq(_property_error_expected(model, "item.height"), EAGAIN);

   ret__ = efl_loop_begin(efl_app_main_get(EFL_APP_CLASS));
   real__ = efl_loop_exit_code_process(ret__);
   fail_if(real__ != 0);

   ck_assert_int_eq(_property_uint_expected(model, "total.width"), base_ints[0] * 2);
   ck_assert_int_eq(_property_uint_expected(model, "total.height"), base_ints[0] * 3 * 3);
   ck_assert_int_eq(_property_uint_expected(model, "item.width"), base_ints[0] * 2);
   ck_assert_int_eq(_property_uint_expected(model, "item.height"), base_ints[0] * 3);
}
EFL_END_TEST

void
efl_ui_model(TCase *tc)
{
   tcase_add_test(tc, efl_ui_model_homogeneous_test);
}
